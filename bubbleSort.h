#ifndef BUBBLESORT_H
#define BUBBLESORT_H

#include <cstddef>

/*
 * bubbleSort
 * ---------------------------------------------------------------------------
 * A simple, easy-to-understand sorting routine that repeatedly steps through
 * the list, compares adjacent pairs, and swaps them if they are in the wrong
 * order. It "bubbles" larger elements toward the end of the array on each
 * pass.
 *
 * Notes for the reader:
 * - This implementation is in-place and does not allocate extra memory.
 * - It's a stable sort (equal elements keep their relative order).
 * - Time complexity: O(n^2) in the worst and average cases. Good only for
 *   very small input sizes or educational purposes.
 *
 * Parameters:
 *   A: pointer to the array to sort (modified in-place)
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
