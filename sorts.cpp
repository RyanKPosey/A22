#include "sorts.h"
#include <vector>
#include <algorithm>

static void merge(std::vector<int>& a, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    std::vector<int> L(n1);
    std::vector<int> R(n2);
    for (int i = 0; i < n1; ++i) L[i] = a[left + i];
    for (int j = 0; j < n2; ++j) R[j] = a[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) a[k++] = L[i++];
        else a[k++] = R[j++];
    }
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];
}

static void mergeSortRec(std::vector<int>& a, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortRec(a, left, mid);
    mergeSortRec(a, mid + 1, right);
    merge(a, left, mid, right);
}

void mergeSort(std::vector<int>& a) {
    if (!a.empty()) mergeSortRec(a, 0, (int)a.size() - 1);
}

void selectionSort(std::vector<int>& a) {
    int n = (int)a.size();
    for (int i = 0; i < n - 1; ++i) {
        int minIdx = i;
        for (int j = i + 1; j < n; ++j)
            if (a[j] < a[minIdx]) minIdx = j;
        if (minIdx != i) std::swap(a[i], a[minIdx]);
    }
}

void bubbleSort(std::vector<int>& a) {
    int n = (int)a.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = n - 1; j > i; --j) {
            if (a[j] < a[j - 1]) std::swap(a[j], a[j - 1]);
        }
    }
}

void insertionSort(std::vector<int>& a) {
    int n = (int)a.size();
    for (int i = 1; i < n; ++i) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = key;
    }
}

bool isSorted(const std::vector<int>& a) {
    for (size_t i = 1; i < a.size(); ++i) if (a[i-1] > a[i]) return false;
    return true;
}
