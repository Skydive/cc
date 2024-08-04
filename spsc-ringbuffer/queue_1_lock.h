#pragma once

#include <mutex>
#include "queue_base.h"

using namespace std;
template<class T>
class QueueV1 : public QueueBase<T> {
public:
    size_t capacity_;
    size_t read_idx_, write_idx_;
    T* buffer_;

    mutex mutex_;

    QueueV1(size_t capacity) : capacity_(capacity), read_idx_(0), write_idx_(0) {
        buffer_ = new T[capacity];
    };
    
    ~QueueV1() {
        while (front())pop();
        delete[] buffer_;
    }

    template<typename... Args>
    bool emplace(Args&&... args) {
        if (full())return false;
        const lock_guard<mutex> lock(mutex_);
        new (&buffer_[write_idx_ % capacity_])  T(std::forward<Args>(args)...);
        write_idx_++;
        return true;
    }
    
    // pop and front with mutex

    void pop() { // ensure queue is nonempty
        const lock_guard<mutex> lock(mutex_);
        read_idx_++;
    }
    
    T* front() {
        const lock_guard<mutex> lock(mutex_);
        if (empty())return nullptr;
        return &buffer_[read_idx_ % capacity_];
    }

    inline bool full() const {
        return (write_idx_ - read_idx_) >= capacity_;
    }

    inline bool empty() const {
        return write_idx_ == read_idx_;
    }

    inline size_t capacity() const {
        return capacity_;
    }
};
