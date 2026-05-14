#ifndef INSERTIONSORT_H
#define INSERTIONSORT_H

#include <cstddef>

/* Function: insertionSort
 * ---------------------------------------------------------------------------
 * Sorts an array of long long integers using the Insertion Sort algorithm.
 * 
 * Parameters:
 *   A: pointer to the array to sort
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
