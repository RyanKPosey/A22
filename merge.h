#ifndef MERGE_H
#define MERGE_H

#include <cstddef>
#include <vector>

/* Function: merge
 * ---------------------------------------------------------------------------
 * Merges two sorted subarrays A[p:q] and A[q+1:r] into a single sorted array.
 * 
 * Parameters:
 *   A: pointer to the array containing both subarrays
 *   p: starting index of first subarray
 *   q: ending index of first subarray
 *   r: ending index of second subarray
 */
void merge(long long int A[], int p, int q, int r) {
	// n_L = q - p + 1          // length of A[p:q]
	int n_L = q - p + 1;
	// n_R = r - q              // length of A[q+1:r]
	int n_R = r - q;
	
	// let L[0:n_L - 1] and R[0:n_R - 1] be new arrays
	std::vector<long long int> L(n_L);
	std::vector<long long int> R(n_R);
	
	// for i = 0 to n_L - 1     // copy A[p:q] into L[0:n_L - 1]
	for (int i = 0; i < n_L; i++) {
		L[i] = A[p + i];
	}
	
	// for j = 0 to n_R - 1     // copy A[q+1:r] into R[0:n_R - 1]
	for (int j = 0; j < n_R; j++) {
		R[j] = A[q + 1 + j];
	}
	
	// i = 0                    // i indexes the smallest remaining element in L
	int i = 0;
	// j = 0                    // j indexes the smallest remaining element in R
	int j = 0;
	// k = p                    // k indexes the location in A to fill
	int k = p;
	
	// As long as each of the arrays L and R contains an unmerged element,
	// copy the smallest unmerged element back into A[p:r].
	// while i < n_L and j < n_R
	while (i < n_L && j < n_R) {
		// if L[i] <= R[j]
		if (L[i] <= R[j]) {
			// A[k] = L[i]
			A[k] = L[i];
			// i = i + 1
			i++;
		}
		// else A[k] = R[j]
		else {
			A[k] = R[j];
			// j = j + 1
			j++;
		}
		// k = k + 1
		k++;
	}
	
	// Having gone through one of L and R entirely, copy the
	// remainder of the other to the end of A[p:r].
	// while i < n_L
	while (i < n_L) {
		// A[k] = L[i]
		A[k] = L[i];
		// i = i + 1
		i++;
		// k = k + 1
		k++;
	}
	
	// while j < n_R
	while (j < n_R) {
		// A[k] = R[j]
		A[k] = R[j];
		// j = j + 1
		j++;
		// k = k + 1
		k++;
	}
}

/* Function: mergeSortHelper
 * ---------------------------------------------------------------------------
 * Recursive helper function for merge sort.
 * Sorts A[p:r] in place.
 * 
 * Parameters:
 *   A: pointer to the array to sort
 *   p: starting index of subarray to sort
 *   r: ending index of subarray to sort
 */
void mergeSortHelper(long long int A[], int p, int r) {
	// if p >= r
	if (p >= r) {
		// return              // zero or one element?
		return;
	}
	
	// q = ⌊(p + r)/2⌋          // midpoint of A[p:r]
	int q = (p + r) / 2;
	
	// MERGE-SORT(A, p, q)      // recursively sort A[p:q]
	mergeSortHelper(A, p, q);
	
	// MERGE-SORT(A, q + 1, r)  // recursively sort A[q + 1:r]
	mergeSortHelper(A, q + 1, r);
	
	// Merge A[p:q] and A[q + 1:r] into A[p:r].
	// MERGE(A, p, q, r)
	merge(A, p, q, r);
}

/* Function: mergeSort
 * ---------------------------------------------------------------------------
 * Sorts an array of long long integers using the Merge Sort algorithm.
 * 
 * Parameters:
 *   A: pointer to the array to sort
 *   n: number of elements in the array
 */
void mergeSort(long long int A[], int n) {
	if (n > 0) {
		mergeSortHelper(A, 0, n - 1);
	}
}

#endif
