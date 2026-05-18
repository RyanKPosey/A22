#ifndef MERGE_H
#define MERGE_H

#include <cstddef>
#include <vector>

/*
 * merge
 * ---------------------------------------------------------------------------
 * Helper that takes two adjacent sorted ranges inside the same array
 * (A[p..q] and A[q+1..r]) and merges them into a single sorted range
 * stored back into A[p..r]. This is the classic merge step used by
 * merge sort.
 *
 * Notes for the reader:
 * - This function allocates temporary vectors to hold the left and right
 *   halves while merging. That makes the merge step simple and safe.
 * - The merge is stable: equal elements from the left half precede
 *   equal elements from the right half.
 * - The work done is linear in the total number of elements merged.
 *
 * Parameters:
 *   A: pointer to the array containing both subarrays (modified in-place)
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

/*
 * mergeSortHelper
 * ---------------------------------------------------------------------------
 * The recursive part of merge sort. It splits the given range in half,
 * recursively sorts each half, then merges them back together using
 * the `merge` helper above.
 *
 * Notes for the reader:
 * - Recursion depth is O(log n). Each level performs linear work,
 *   so the overall time complexity is O(n log n).
 * - Merge sort requires O(n) auxiliary space because of the temporary
 *   arrays used during merging.
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

/*
 * mergeSort
 * ---------------------------------------------------------------------------
 * Top-level entry point for merge sort. Sorts the array A[0..n-1] in place
 * by delegating to the recursive helper.
 *
 * Notes for the reader:
 * - Use this when you need a reliable O(n log n) sort with stable output.
 * - It uses extra memory proportional to the input size (for merges).
 */
void mergeSort(long long int A[], int n) {
	if (n > 0) {
		mergeSortHelper(A, 0, n - 1);
	}
}

#endif
