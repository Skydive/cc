#pragma once

#include <atomic>
// size_t
#include <cstddef>

#include "queue_base.h"

using namespace std;
template<class T>
class QueueV3 : public QueueBase<T> {
public:
    
    // alignas(hardware_destructive_interference_size)
    atomic<size_t> read_idx_;
    
    // alignas(hardware_destructive_interference_size)
    atomic<size_t> write_idx_;

    // alignas(hardware_destructive_interference_size)
    size_t capacity_;
    T* buffer_;

    static_assert(atomic<size_t>::is_always_lock_free);

    QueueV3(size_t capacity) : capacity_(capacity), read_idx_(0), write_idx_(0) {
        // Make capacity a power of 2
        buffer_ = new T[capacity];
    };

    ~QueueV3() {
        while (front())pop();
        delete[] buffer_;
    }


    template<typename... Args>
    bool emplace(Args&& ...args) noexcept {
        size_t write_idx = write_idx_.load(memory_order_relaxed);
        size_t read_idx = read_idx_.load(memory_order_acquire);
        if (full(read_idx, write_idx))return false;
        // write_idx atomic
        write_idx = write_idx_.load(memory_order_acquire); // memory_order_seq_cst
        new (&buffer_[write_idx % capacity_]) T(std::forward<Args>(args)...);
        write_idx_.store(write_idx + 1, memory_order_release);
        return true;
    }
    
    T* front() noexcept {
        size_t write_idx = write_idx_.load(memory_order_acquire);
        size_t read_idx = read_idx_.load(memory_order_acquire);
        if (empty(read_idx, write_idx))return nullptr;
        return &buffer_[read_idx % capacity_];
    }

    void pop() noexcept { // ensure queue is nonempty
        const size_t read_idx = read_idx_.load(memory_order_relaxed);
        assert(write_idx_.load(memory_order_acquire) != read_idx &&
           "Can only call pop() after front() has returned a non-nullptr");
        buffer_[read_idx % capacity_].~T();
        read_idx_.store(read_idx + 1, memory_order_release);
    }

    // Overloaded
    inline bool full(size_t read_idx, size_t write_idx) const noexcept {
        return (write_idx - read_idx) >= capacity_;
    }

    inline bool empty(size_t read_idx, size_t write_idx) const noexcept {
        return write_idx == read_idx;
    }

    inline bool full() const noexcept {
        return (write_idx_.load(memory_order_acquire) - read_idx_.load(memory_order_acquire)) >= capacity_;
    }

    inline bool empty() const noexcept {
        return write_idx_.load(memory_order_acquire) == read_idx_.load(memory_order_acquire);
    }

    virtual size_t capacity() const {
        return capacity_;
    }
};
