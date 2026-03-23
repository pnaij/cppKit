//
// Created by jianp on 2026/3/23.
//

#include <iostream>
#include <memory>

template<typename T>
class Ref_count {
private:
    T* ptr;
    int* count;

public:

    Ref_count(T* t) : ptr(t), count(new int(1)) {

    }

    ~Ref_count() {
        decrease();
    }

    Ref_count(const Ref_count<T>& other) {
        count = other.count;
        ptr = other.ptr;
        increase();
    }

    Ref_count<T>& operator=(const Ref_count<T>& other) {
        if(other != this) {
            decrease();
            ptr = other.ptr;
            count = other.count;
            increase();
        }
        return *this;
    }

    T* operator->() const {
        return ptr;
    }

    T& operator*() const {
        return *ptr;
    }

    void increase() {
        if(count) {
            (*count)++;
        }
    }

    void decrease() {
        if(count) {
            (*count)--;
            if((*count) == 0) {
                delete ptr;
                ptr = nullptr;
                delete count;
                count = nullptr;
            }
        }
    }

    T* get() const {
        return ptr;
    }

    int get_count() const {
        if(!count) {
            return 0;
        }

        return *count;
    }
};
