#ifndef MINHEAP_H
#define MINHEAP_H

#include "SimpleVector.h"

template <typename T>
struct MinHeap {
    SimpleVector<T> heap;

    int parent(int i) { return (i - 1) / 2; }
    int left(int i) { return (2 * i) + 1; }
    int right(int i) { return (2 * i) + 2; }

    void push(T val) {
        heap.push_back(val);
        int index = heap.size() - 1;
        bubbleUp(index);
    }

    void pop() {
        if (heap.size() == 0) return;
        heap[0] = heap[heap.size() - 1];
        heap.pop_back();
        if (heap.size() > 0) {
            bubbleDown(0);
        }
    }

    T top() {
        if (heap.size() > 0) return heap[0];
        return T();
    }

    bool empty() { return heap.empty(); }
    int size() { return heap.size(); }

    void bubbleUp(int i) {
        while (i > 0 && heap[i] < heap[parent(i)]) {
            T temp = heap[i];
            heap[i] = heap[parent(i)];
            heap[parent(i)] = temp;
            i = parent(i);
        }
    }

    void bubbleDown(int i) {
        int minIndex = i;
        int n = heap.size();

        while (true) {
            int l = left(i);
            int r = right(i);
            minIndex = i;

            if (l < n && heap[l] < heap[minIndex]) {
                minIndex = l;
            }

            if (r < n && heap[r] < heap[minIndex]) {
                minIndex = r;
            }

            if (minIndex != i) {
                T temp = heap[i];
                heap[i] = heap[minIndex];
                heap[minIndex] = temp;
                i = minIndex;
            } else {
                break;
            }
        }
    }
};

#endif
