#pragma once

#include <atomic>
// size_t
#include <cstddef>

#include "queue_base.h"

#include <bitset>

using namespace std;
template<class T>
class QueueV4 : public QueueBase<T> {
public:
    static constexpr size_t kCacheLineSize =
      std::hardware_destructive_interference_size;

    alignas(kCacheLineSize)
    atomic<size_t> read_idx_ = {0};
    
    alignas(kCacheLineSize)
    size_t read_idx_cache_ = 0;

    alignas(kCacheLineSize)
    atomic<size_t> write_idx_ = {0};

    alignas(kCacheLineSize)
    size_t write_idx_cache_ = 0;


    alignas(kCacheLineSize)
    size_t capacity_;
    size_t mask_;
    

    alignas(kCacheLineSize)
    T* buffer_;


    static_assert(atomic<size_t>::is_always_lock_free);

    QueueV4(size_t capacity) {
        // Make capacity a power of 2
        // make capacity largest power of 2 smaller than it
        size_t capacityp2 = 1 << (64 - __builtin_clzll(capacity) - 1);

        // check power of 2
        assert(capacityp2 <= capacity);
        assert((capacityp2 & (capacityp2 - 1)) == 0);
        capacity_ = capacityp2;
        mask_ = capacity_ - 1;
        buffer_ = new T[capacity_];

        static_assert(alignof(QueueV4<T>) == kCacheLineSize, "");
        static_assert(sizeof(QueueV4<T>) >= 3 * kCacheLineSize, "");
    };

    ~QueueV4() {
        while (front())pop();
        delete[] buffer_;
    }


    template<typename... Args>
    bool emplace(Args&& ...args) noexcept {
        const size_t write_idx = write_idx_.load(memory_order_relaxed);
        size_t next_write_idx = write_idx + 1;

        if((next_write_idx & mask_) == (read_idx_cache_ & mask_)) { // Could we be full? (relaxed)
            read_idx_cache_ = read_idx_.load(memory_order_acquire); // Acquire to check for sure... --- and cache it
            if ((next_write_idx & mask_) == (read_idx_cache_ & mask_)) { // We are full!
                return false;
            }
        }
        // write_idx atomic
        new (&buffer_[write_idx & mask_]) T(std::forward<Args>(args)...);
        write_idx_.store(next_write_idx, memory_order_release);
        return true;
    }
    
    T* front() noexcept {
        const size_t read_idx = read_idx_.load(memory_order_relaxed);
        size_t next_read_idx = read_idx + 1;
        if((read_idx & mask_) == (write_idx_cache_ & mask_)) { // Could we be empty? (relaxed)
            write_idx_cache_ = write_idx_.load(memory_order_acquire); // Acquire to check for sure... --- and cache it
            if ((read_idx & mask_) == (write_idx_cache_ & mask_)) { // We are empty!
                return nullptr;
            }
        }
        return &buffer_[read_idx & mask_];
    }

    void pop() noexcept { // ensure queue is nonempty
        const size_t read_idx = read_idx_.load(memory_order_relaxed);
        assert(write_idx_.load(memory_order_acquire) != read_idx &&
           "Can only call pop() after front() has returned a non-nullptr");
        buffer_[read_idx & mask_].~T();
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
