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
    virtual bool enqueue(const T& data) override {
        if (full())return false;

        const lock_guard<mutex> lock(mutex_);
        
        new (&buffer_[write_idx_ % capacity_]) T(data);
        write_idx_++;
        return true;
    }
    virtual bool dequeue(T* data) override {
        if (empty())return false;
        
        const lock_guard<mutex> lock(mutex_);

        *data = buffer_[read_idx_ % capacity_];
        read_idx_++;
        return true;
    }
    
    virtual T* front() override {
        if (empty())return nullptr;
        return &buffer_[read_idx_ % capacity_];
    }

    inline bool full() const override {
        return ((write_idx_ + 1) % capacity_) == read_idx_;
    }

    inline bool empty() const override {
        return write_idx_ == read_idx_;
    }

    virtual size_t capacity() const override {
        return capacity_;
    }
};
