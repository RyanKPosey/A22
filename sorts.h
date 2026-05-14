// Sorting algorithms header
#ifndef SORTS_H
#define SORTS_H

#include <vector>

void selectionSort(std::vector<int>& a);
void bubbleSort(std::vector<int>& a);
void insertionSort(std::vector<int>& a);
void mergeSort(std::vector<int>& a);

bool isSorted(const std::vector<int>& a);

#endif // SORTS_H
