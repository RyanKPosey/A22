#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <algorithm>
#include "sorts.h"

using clk = std::chrono::high_resolution_clock;

static std::vector<int> make_random(int n, int seed = 12345) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, n);
    std::vector<int> a(n);
    for (int i = 0; i < n; ++i) a[i] = dist(rng);
    return a;
}

static void run_and_report(const std::string& name, std::vector<int> a) {
    auto start = clk::now();
    if (name == "selection") selectionSort(a);
    else if (name == "bubble") bubbleSort(a);
    else if (name == "insertion") insertionSort(a);
    else if (name == "merge") mergeSort(a);
    auto end = clk::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << name << " sort: " << ms << " ms";
    std::cout << " | sorted=" << (isSorted(a) ? "yes" : "NO") << '\n';
}

static void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [option] [size]\n";
    std::cout << "Options:\n";
    std::cout << "  --selection | -s    run selection sort\n";
    std::cout << "  --bubble    | -b    run bubble sort\n";
    std::cout << "  --insertion | -i    run insertion sort\n";
    std::cout << "  --merge     | -m    run merge sort\n";
    std::cout << "  --all       | -a    run all sorts (copies array for each)\n";
    std::cout << "Default size: 10000\n";
}

int main(int argc, char** argv) {
    if (argc < 2) { print_usage(argv[0]); return 1; }
    std::string opt = argv[1];
    int n = 10000;
    if (argc >= 3) n = std::stoi(argv[2]);

    if (opt == "-a" || opt == "--all") {
        auto base = make_random(n);
        run_and_report("selection", base);
        run_and_report("bubble", base);
        run_and_report("insertion", base);
        run_and_report("merge", base);
        return 0;
    }

    auto a = make_random(n);
    if (opt == "-s" || opt == "--selection") run_and_report("selection", a);
    else if (opt == "-b" || opt == "--bubble") run_and_report("bubble", a);
    else if (opt == "-i" || opt == "--insertion") run_and_report("insertion", a);
    else if (opt == "-m" || opt == "--merge") run_and_report("merge", a);
    else { print_usage(argv[0]); return 1; }
    return 0;
}
