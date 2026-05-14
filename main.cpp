#include <algorithm>			// Needed for sort
#include <atomic>				// Needed for heap_track counters
#include "bubbleSort.h"		// Needed for bubbleSort
#include <chrono>				// Needed to time and uint64 definition
#include <cstdlib>				// Needed for std::malloc / std::free in operator new
#include <cstdio>				// Needed for snprintf
#include <dirent.h>			// Needed to read directories
#include "insertionSort.h"	// Needed for insertionSort
#include <iomanip>			// Needed for setw
#include <iostream>			// Needed for cout
#include <limits>				// Needed for numeric_limits (input validation)
#include "live_monitor.h"		// Needed for LiveMonitor (htop-style live bars)
#include <malloc.h>			// Needed for malloc_usable_size
#include "merge.h"			// Needed for mergeSort
#include <pwd.h>		// Needed for struct passwd
#include <sys/stat.h>		// Needed for struct stat
#include "sysmem.h"			// Needed for human_bytes (used by grader)
#include <string.h>			// Needed for strcmp, strcat

using namespace std;

using namespace std::chrono;

// =============================================================================
// Heap tracking -- counts every C++ allocation that goes through operator new
// / operator delete (including std::vector inside merge.h).  Does NOT track
// raw malloc/calloc, which is exactly what we want -- the studentArray is
// sized separately via malloc_usable_size().
// =============================================================================
namespace heap_track {
	std::atomic<size_t> live{0};	// currently outstanding bytes
	std::atomic<size_t> peak{0};	// high-water mark since last reset
	std::atomic<size_t> total{0};	// cumulative bytes ever allocated since reset
	/* Per-thread on/off switch.  The sampler thread inside LiveMonitor sets
	 * this to false so its own ifstream / string formatting allocations
	 * don't pollute the algorithm's measurements.  Default is true so the
	 * main thread (where sort allocations happen) is always counted. */
	thread_local bool tracking_enabled = true;
	void reset() { live = 0; peak = 0; total = 0; }
}

void* operator new(std::size_t n) {
	void* p = std::malloc(n + sizeof(std::size_t));
	if (!p) throw std::bad_alloc();
	*static_cast<std::size_t*>(p) = n;

	if (heap_track::tracking_enabled) {
		size_t now  = (heap_track::live += n);
		size_t prev = heap_track::peak.load(std::memory_order_relaxed);
		while (now > prev && !heap_track::peak.compare_exchange_weak(prev, now)) {}
		heap_track::total += n;
	}

	return static_cast<char*>(p) + sizeof(std::size_t);
}
void operator delete(void* p) noexcept {
	if (!p) return;
	char* base = static_cast<char*>(p) - sizeof(std::size_t);
	size_t n = *reinterpret_cast<std::size_t*>(base);
	if (heap_track::tracking_enabled) {
		heap_track::live -= n;
	}
	std::free(base);
}
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }


namespace {
  /* Adds commas to a numeric value to make it easier to read. */
  template <typename Integer> string addCommasTo(Integer n) {
    if (n < 0) return "-" + addCommasTo(-n);

    string result;
    for (size_t i = 0; n >= 10; i++) {
      result += static_cast<char>('0' + n % 10);
      n /= 10;
      if (i % 3 == 2) result += ',';
    }
    result += static_cast<char>('0' + n);
    reverse(result.begin(), result.end());
    return result;
  }
}

/* Three sorts only -- selection sort has been retired. */
enum algorithm {BUBBLE, INSERTION, MERGE};

/* Holds the user's choice of algorithm(s) to run.  When run_all is true,
 * `which` is ignored and run_test iterates BUBBLE..MERGE. */
struct AlgoChoice {
	bool      run_all;
	algorithm which;
};

struct FileData {
	int num_elems;
	long long int* elems;
};

#define DATA_DIRECTORY "./data/"

#define error(msg) do_error(msg, __FILE__, __LINE__)
static void do_error(const char* msg, const char* file, unsigned line) {
	fprintf(stderr, "ERROR: %s\nFile: %s\nLine: %u\n", msg, file, line);
	abort();
}

static struct FileData read_file(const char* test_file) {
	struct FileData result;

	FILE* f = fopen(test_file, "r");
	if (f == NULL)
		error("Can't open data file.");

	if (fscanf(f, "%lld", &result.num_elems) != 1)
		error("Failed to read number of elements from data file.");

	result.elems = static_cast<long long int*>(calloc(result.num_elems, sizeof(long long int)));

	for (size_t i = 0; i < (size_t)result.num_elems; i++) {
		if (fscanf(f, "%lld", &result.elems[i]) != 1)
			error("Failed to read long long integer from file.");
	}

	if (fscanf(f, "%*lld") != EOF)
		error("Unexpected file contents found after end of data.");

	fclose(f);
	return result;
}

int compare_ints(const void *left, const void *right) {
	if ( *(long long int*)left < *(long long int*)right) return -1;
	if ( *(long long int*)left > *(long long int*)right) return +1;
	return 0;
}

static void assert_sets_equal(const long long int data1[], size_t size1,
                              const long long int data2[], size_t size2) {
	(void)size2;
	for (size_t i = 0; i < size1; i++) {
		if (data1[i] != data2[i]) {
			fprintf(stderr,
				"Mismatch found: answer key[%zu] = %lld and student array[%zu] = %lld\n",
				i, data1[i], i, data2[i]);
			abort();
		}
	}
}


// =============================================================================
// Layout: 66 chars between "= " and " ="
//   sort_name=20, grade=8, time=16, input=11, aux=11.  Total = 66.
// =============================================================================
static const int COL_NAME  = 20;
static const int COL_GRADE = 8;
static const int COL_TIME  = 16;
static const int COL_INPUT = 11;
static const int COL_AUX   = 11;

static void print_sort_header(const string& sorting_algorithm) {
	cout << "= "
	     << setw(COL_NAME)  << left  << sorting_algorithm
	     << setw(COL_GRADE) << right << "GRADE"
	     << setw(COL_TIME)  << right << "HH:MM:SS:mmm"
	     << setw(COL_INPUT) << right << "INPUT"
	     << setw(COL_AUX)   << right << "AUX"
	     << " =" << endl;
}

void grader(const string& grade, uint64_t duration,
            size_t input_bytes, size_t aux_bytes) {
	uint64_t hours = duration / 3600000;  duration -= hours   * 3600000;
	uint64_t minutes = duration / 60000;  duration -= minutes * 60000;
	uint64_t seconds = duration / 1000;   duration -= seconds * 1000;

	char timebuf[24];
	snprintf(timebuf, sizeof(timebuf), "%02llu:%02llu:%02llu:%03llu",
	         (unsigned long long)hours,
	         (unsigned long long)minutes,
	         (unsigned long long)seconds,
	         (unsigned long long)duration);

	cout << "= "
	     << setw(COL_NAME)  << left  << ""
	     << setw(COL_GRADE) << right << grade
	     << setw(COL_TIME)  << right << timebuf
	     << setw(COL_INPUT) << right << human_bytes(input_bytes)
	     << setw(COL_AUX)   << right << human_bytes(aux_bytes)
	     << " =" << endl;
}


static size_t select_file(const string fileNames[], size_t numFiles) {
	if (numFiles == 0)
		error("No data files available to choose from.");

	cout << "= " << setw(66) << setfill('-') << "-" << setfill(' ') << " =" << endl;
	cout << "= " << setw(66) << left << "AVAILABLE DATA SETS:" << " =" << endl;

	for (size_t i = 0; i < numFiles; i++) {
		size_t slash = fileNames[i].find_last_of('/');
		string display = (slash == string::npos)
				? fileNames[i]
				: fileNames[i].substr(slash + 1);
		string line = "  " + to_string(i + 1) + ") " + display;
		cout << "= " << setw(66) << left << line << " =" << endl;
	}

	cout << "= " << setw(66) << setfill('-') << "-" << setfill(' ') << " =" << endl;

	size_t choice = 0;
	while (true) {
		cout << "Select a data set [1-" << numFiles << "]: " << flush;
		if (cin >> choice && choice >= 1 && choice <= numFiles) {
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			break;
		}
		cout << "Invalid selection. Please enter a number between 1 and "
		     << numFiles << "." << endl;
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}

	return choice - 1;
}


/* Function: select_algorithm()
 * ---------------------------------------------------------------------------
 * Secondary menu: which sort to run, or all three for grade submission.
 */
static AlgoChoice select_algorithm() {
	cout << "= " << setw(66) << setfill('-') << "-" << setfill(' ') << " =" << endl;
	cout << "= " << setw(66) << left << "SORTING ALGORITHM:"                  << " =" << endl;
	cout << "= " << setw(66) << left << "  1) Bubble Sort"                    << " =" << endl;
	cout << "= " << setw(66) << left << "  2) Insertion Sort"                 << " =" << endl;
	cout << "= " << setw(66) << left << "  3) Merge Sort"                     << " =" << endl;
	cout << "= " << setw(66) << left << "  4) Run all (final grade submission)" << " =" << endl;
	cout << "= " << setw(66) << setfill('-') << "-" << setfill(' ') << " =" << endl;

	int choice = 0;
	while (true) {
		cout << "Select an algorithm [1-4]: " << flush;
		if (cin >> choice && choice >= 1 && choice <= 4) {
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			break;
		}
		cout << "Invalid selection. Please enter a number between 1 and 4." << endl;
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}

	AlgoChoice result{false, BUBBLE};
	switch (choice) {
		case 1: result.which   = BUBBLE;    break;
		case 2: result.which   = INSERTION; break;
		case 3: result.which   = MERGE;     break;
		case 4: result.run_all = true;      break;
	}
	return result;
}


/* Function: run_one(a, fileNames, numFiles)
 * ---------------------------------------------------------------------------
 * Runs a single sorting algorithm against every file in the provided list.
 *
 * Layout per file:
 *     =  ELEMENTS:    10,000  SORTING                         <-- transient
 *     =                       PASSED   00:00:00:012   78.1K   78.1K =
 *     Mem [...]
 *     Swp [...]
 *     Heap[...]
 *
 * The PASSED line is reserved as a blank placeholder before the bars start;
 * after the sort finishes we rewind the cursor up to that placeholder, fill
 * it in with the real grader output, then move the cursor back below the
 * bars so subsequent output (next sort, shell prompt) lands cleanly.
 */
static void run_one(algorithm a, string fileNames[], size_t numFiles) {
	string sorting_algorithm;
	switch(a){
		case BUBBLE:    sorting_algorithm = "BUBBLE SORT";    break;
		case INSERTION: sorting_algorithm = "INSERTION SORT"; break;
		case MERGE:     sorting_algorithm = "MERGE SORT";     break;
		default:        error("Invalid Sorting Algorithm chosen");
	}

	cout << "= " << setw(66) << setfill('-') << "-" << setfill(' ') << " =" << endl;
	print_sort_header(sorting_algorithm);
	cout << "= " << setw(66) << setfill('-') << "-" << setfill(' ') << " =" << endl;

	for (size_t i = 0; i < numFiles; i++){
		cout << "=  " << setw(10) << left << "ELEMENTS:" << flush;

		char* data_file = new char[fileNames[i].length() + 1];
		data_file[fileNames[i].length()] = '\0';
		for (size_t j = 0; j < fileNames[i].length(); j++)
			data_file[j] = fileNames[i][j];

		cout << " READING FILE" << flush;
		struct FileData data = read_file(data_file);
		cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b" << flush;
		cout << setw(10) << right << addCommasTo(data.num_elems) << flush;

		long long int* studentArray = static_cast<long long int*>(
			calloc(data.num_elems, sizeof(long long int)));

		size_t input_bytes = malloc_usable_size(studentArray);

		for (size_t k = 0; k < (size_t)data.num_elems; k++) {
			studentArray[k] = data.elems[k];
		}

		cout << "  SORTING\n" << flush;

		/* Reserve a blank placeholder line for the PASSED result --
		 * we'll rewind and fill it in after the sort completes. */
		cout << "\n" << flush;

		heap_track::reset();
		LiveMonitor monitor(64, 30, input_bytes);
		monitor.start();

		uint64_t b4 = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();

		switch(a){
			case BUBBLE:    bubbleSort   (studentArray, data.num_elems); break;
			case INSERTION: insertionSort(studentArray, data.num_elems); break;
			case MERGE:     mergeSort    (studentArray, data.num_elems); break;
		}

		uint64_t after = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();

		uint64_t duration = after - b4;

		monitor.stop();

		size_t aux_peak = heap_track::peak.load();

		qsort(data.elems, data.num_elems, sizeof(long long int), compare_ints);
		assert_sets_equal(data.elems, data.num_elems, studentArray, data.num_elems);

		/* Cursor is currently 4 lines below the PASSED placeholder
		 * (1 placeholder + 3 bar lines).  Rewind, write the result, then
		 * move back below the bars so the next iteration / next sort
		 * starts on a clean line. */
		cout << "\033[4A\033[2K";              // up 4 lines, clear line
		grader("PASSED", duration, input_bytes, aux_peak);  // ends with endl
		cout << "\033[3B" << flush;            // back down past the bars

		free(studentArray);
		free(data.elems);
		delete[] data_file;
	}
}


static void run_test(string fileNames[], size_t numFiles, passwd *pw,
                     AlgoChoice choice) {
	(void)pw;
	if (choice.run_all) {
		for (algorithm a = BUBBLE; a <= MERGE; a = static_cast<algorithm>(a + 1))
			run_one(a, fileNames, numFiles);
	} else {
		run_one(choice.which, fileNames, numFiles);
	}
}


void grade_header(string assignment, passwd *pw){
	cout << "\033[H\033[2J" << flush; //Clear the Screen
	cout << setw(71) << setfill('=') << "\n" << setfill(' ');
	cout << setw(69) << left << "= CS1C AUTO GRADER" << "=" << endl;
	cout << "= ASSIGNMENT: " << setw(55) << left << assignment << "=" << endl;
	time_t now = time(0);
	char* dt = ctime(&now);
	cout << "= LOCAL DATE AND TIME: " << setw(25) << left << dt;
	cout << "= " << setw(66) << setfill('-') << "-" << setfill(' ') << " =" << endl;
	return;
}

int main() {
	size_t numFiles = 0;
	struct stat info;
	struct passwd *pw = NULL;

	DIR* dataFiles = opendir(DATA_DIRECTORY);
	if (dataFiles == NULL)
		error("Could not open " DATA_DIRECTORY " for reading.");

	for (struct dirent* entry; (entry = readdir(dataFiles)) != NULL; ) {
		if (entry->d_name[0] == '.')
			continue;
		numFiles++;
	}
	closedir(dataFiles);
	string fileNames[numFiles];
	size_t counter = 0;

	dataFiles = opendir(DATA_DIRECTORY);
	if (dataFiles == NULL)
		error("Could not open " DATA_DIRECTORY " for reading.");

	char owner[numFiles][20] {};

	for (struct dirent* entry; (entry = readdir(dataFiles)) != NULL; ) {
		if (entry->d_name[0] == '.')
			continue;

		char test_file_name[strlen(entry->d_name) + strlen(DATA_DIRECTORY) + 1];
		strcpy(test_file_name, DATA_DIRECTORY);
		strcat(test_file_name, entry->d_name);
		fileNames[counter] = test_file_name;

		stat(test_file_name, &info);
		pw = getpwuid(info.st_uid);

		counter++;
	}
	if (errno != 0)
		error("Error traversing the " DATA_DIRECTORY " directory.");

	closedir(dataFiles);

	grade_header("A22", pw);

	std::sort(fileNames, fileNames + numFiles);

	/* 1. Pick which data set to run against. */
	size_t chosen = select_file(fileNames, numFiles);
	string selectedFile[1] = { fileNames[chosen] };

	/* 2. Pick which algorithm(s) to run. */
	AlgoChoice algoChoice = select_algorithm();

	/* 3. Run. */
	run_test(selectedFile, 1, pw, algoChoice);

	return 0;
}
