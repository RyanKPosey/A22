#ifndef INSERTIONSORT_H
#define INSERTIONSORT_H

#include <cstddef>

/*
 * insertionSort
 * ---------------------------------------------------------------------------
 * A straightforward sorting method that builds the final sorted list one
 * element at a time. For each element, it inserts that element into the
 * already-sorted portion to its left.
 *
 * Notes for the reader:
 * - This is an in-place, stable algorithm with low overhead.
 * - Works very well for small arrays or arrays that are already nearly
 *   sorted (best case O(n)). Worst-case time complexity is O(n^2).
 * - Use this when simplicity and small memory footprint are more important
 *   than raw speed on large inputs.
 *
 * Parameters:
 *   A: pointer to the array to sort (modified in-place)
 *   n: number of elements in the array
 */
void insertionSort(long long int A[], int n) {
	// Start from the second element (index 1)
	for (int i = 1; i < n; i++) {
		long long int key = A[i];
		int j = i - 1;
		
		// Move all elements greater than key one position ahead
		while (j >= 0 && A[j] > key) {
			A[j + 1] = A[j];
			j--;
		}
		// Insert the key at its correct position
		A[j + 1] = key;
	}
}

#endif
