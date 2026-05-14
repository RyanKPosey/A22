#ifndef BUBBLESORT_H
#define BUBBLESORT_H

#include <cstddef>

/* Function: bubbleSort
 * ---------------------------------------------------------------------------
 * Sorts an array of long long integers using the Bubble Sort algorithm.
 * 
 * Parameters:
 *   A: pointer to the array to sort
 *   n: number of elements in the array
 */
void bubbleSort(long long int A[], int n) {
	// for i = 1 to n - 1 (converted to 0-based: i = 0 to n-2)
	for (int i = 0; i < n - 1; i++) {
		// for j = n downto i + 1 (converted to 0-based: j = n-1 down to i+1)
		for (int j = n - 1; j > i; j--) {
			// if A[j] < A[j - 1]
			if (A[j] < A[j - 1]) {
				// exchange A[j] with A[j - 1]
				long long int temp = A[j];
				A[j] = A[j - 1];
				A[j - 1] = temp;
			}
		}
	}
}

#endif
