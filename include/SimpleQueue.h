#ifndef SIMPLEQUEUE_H
#define SIMPLEQUEUE_H

#include "SimpleList.h"

template <typename T>
struct SimpleQueue {
    SimpleList<T> list;

    void push(T val) {
        list.push_back(val);
    }

    void pop() {
        list.pop_front();
    }

    T front() {
        if (list.head != nullptr) {
            return list.head->data;
        }
        return T();  // Return default if empty
    }

    bool empty() {
        return list.empty();
    }

    int size() {
        int count = 0;
        Node<T>* current = list.head;
        while (current != nullptr) {
            count++;
            current = current->next;
        }
        return count;
    }
};

#endif
