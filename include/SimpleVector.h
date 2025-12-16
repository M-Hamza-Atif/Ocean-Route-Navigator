#ifndef SIMPLEVECTOR_H
#define SIMPLEVECTOR_H

template <typename T>
struct SimpleVector {
    T* data;
    int currentSize;
    int capacity;

    SimpleVector() {
        capacity = 10;
        currentSize = 0;
        data = new T[capacity];
    }

    SimpleVector(const SimpleVector& other) {
        capacity = other.capacity;
        currentSize = other.currentSize;
        data = new T[capacity];
        for (int i = 0; i < currentSize; i++) {
            data[i] = other.data[i];
        }
    }

    SimpleVector& operator=(const SimpleVector& other) {
        if (this != &other) {
            delete[] data;
            capacity = other.capacity;
            currentSize = other.currentSize;
            data = new T[capacity];
            for (int i = 0; i < currentSize; i++) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    ~SimpleVector() {
        delete[] data;
    }

    void push_back(const T& value) {
        if (currentSize >= capacity) {
            int newCapacity = capacity * 2;
            T* newData = new T[newCapacity];
            for (int i = 0; i < currentSize; i++) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }
        data[currentSize] = value;
        currentSize++;
    }

    void pop_back() {
        if (currentSize > 0) {
            currentSize--;
        }
    }
    
    bool contains(const T& value) {
        for (int i = 0; i < currentSize; i++) {
            if (data[i] == value) return true;
        }
        return false;
    }

    T& operator[](int index) {
        return data[index];
    }
    
    const T& operator[](int index) const {
        return data[index];
    }

    int size() const {
        return currentSize;
    }

    bool empty() const {
        return currentSize == 0;
    }
    
    void clear() {
        currentSize = 0;
    }
};

#endif