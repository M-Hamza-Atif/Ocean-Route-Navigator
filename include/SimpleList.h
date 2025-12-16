#ifndef SIMPLELIST_H
#define SIMPLELIST_H

template <typename T>
struct Node {
    T data;
    Node* next;

    Node(T val) : data(val), next(nullptr) {}
};

template <typename T>
struct SimpleList {
    Node<T>* head;
    Node<T>* tail;

    SimpleList() {
        head = nullptr;
        tail = nullptr;
    }

    ~SimpleList() {
        Node<T>* current = head;
        while (current != nullptr) {
            Node<T>* nextNode = current->next;
            delete current;
            current = nextNode;
        }
    }

    void push_back(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    void remove(const T& value) {
        if (head == nullptr) return;

        if (head->data == value) {
            Node<T>* temp = head;
            head = head->next;
            if (head == nullptr) tail = nullptr;
            delete temp;
            return;
        }

        Node<T>* current = head;
        while (current->next != nullptr && current->next->data != value) {
            current = current->next;
        }

        if (current->next != nullptr) {
            Node<T>* toDelete = current->next;
            current->next = toDelete->next;
            
            if (toDelete == tail) {
                tail = current;
            }
            delete toDelete;
        }
    }

    Node<T>* begin() {
        return head;
    }
    
    bool empty() {
        return head == nullptr;
    }
    
    void clear() {
        while (head != nullptr) {
            pop_front();
        }
    }
    
    int size() const {
        int count = 0;
        Node<T>* current = head;
        while (current != nullptr) {
            count++;
            current = current->next;
        }
        return count;
    }

    // Remove and return the front element (for queue operations)
    void pop_front() {
        if (head == nullptr) return;
        
        Node<T>* temp = head;
        head = head->next;
        
        // If list became empty, update tail
        if (head == nullptr) {
            tail = nullptr;
        }
        
        delete temp;
    }
};

#endif