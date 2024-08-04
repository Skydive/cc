#pragma once

#include <atomic>
#include "queue_base.h"

using namespace std;
template<class T>
class QueueV2 : public QueueBase<T> {
public:
    
    // alignas(hardware_destructive_interference_size)
    atomic<size_t> read_idx_;
    
    // alignas(hardware_destructive_interference_size)
    atomic<size_t> write_idx_;

    // alignas(hardware_destructive_interference_size)
    size_t capacity_;
    T* buffer_;

    static_assert(atomic<size_t>::is_always_lock_free);

    QueueV2(size_t capacity) : capacity_(capacity), read_idx_(0), write_idx_(0) {
        buffer_ = new T[capacity];
    };

    ~QueueV2() {
        while (front())pop();
        delete[] buffer_;
    }


    template<typename... Args>
    bool emplace(Args&& ...args) noexcept {
        if (full())return false;
        // write_idx atomic
        size_t write_idx = write_idx_.load(); // memory_order_seq_cst
        new (&buffer_[write_idx % capacity_]) T(std::forward<Args>(args)...);
        write_idx_.store(write_idx + 1);
        return true;
    }
    void pop() noexcept { // ensure queue is nonempty
        size_t read_idx = read_idx_.load();
        read_idx_.store(read_idx + 1);
    }
    
    T* front() noexcept {
        if (empty())return nullptr;
        size_t read_idx = read_idx_.load();
        return &buffer_[read_idx % capacity_];
    }

    inline bool full() const noexcept {
        return (write_idx_.load() - read_idx_.load()) >= capacity_;
    }

    inline bool empty() const noexcept {
        return write_idx_.load() == read_idx_.load();
    }

    virtual size_t capacity() const noexcept {
        return capacity_;
    }
};
