// sysctl -w vm.nr_hugepages=512
#pragma once

#include <atomic>
// size_t
#include <cstddef>

#include "queue_base.h"

#include <bitset>

using namespace std;
template<class T, typename Allocator = std::allocator<T>>
class QueueV5 : public QueueBase<T> {
public:
    static_assert(atomic<size_t>::is_always_lock_free);

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
    T* buffer_;


    alignas(kCacheLineSize)
    size_t capacity_;
    size_t mask_;
    Allocator allocator_;

    template <typename Alloc2, typename = void>
    struct has_allocate_at_least : std::false_type {};

    template <typename Alloc2>
    struct has_allocate_at_least<
        Alloc2, std::void_t<typename Alloc2::value_type,
                            decltype(std::declval<Alloc2 &>().allocate_at_least(
                                size_t{}))>> : std::true_type {};

                

    QueueV5(size_t capacity, const Allocator &allocator = Allocator()) : allocator_(allocator){  
        // Make capacity a power of 2
        // make capacity largest power of 2 smaller than it
        size_t capacityp2 = 1 << (64 - __builtin_clzll(capacity) - 1);

        // check power of 2
        assert(capacityp2 <= capacity);
        assert((capacityp2 & (capacityp2 - 1)) == 0);
        capacity_ = capacityp2;
        mask_ = capacity_ - 1;

        size_t hp_num = ((capacity * sizeof(T)) / 1024 / 1024 / 2) + 1;
    
        // buffer_ = new T[capacity_];
        if constexpr (has_allocate_at_least<Allocator>::value) {
            // cout << "QueueV5 AtLeast Allocator" << endl;
            auto res = allocator_.allocate_at_least(capacity_);
            buffer_ = res.ptr;
        } else {
            // cout << "QueueV5 Normal Allocator" << endl;
            buffer_ = std::allocator_traits<Allocator>::allocate(allocator_, capacity_);
        }

        static_assert(alignof(QueueV5<T>) == kCacheLineSize, "");
        static_assert(sizeof(QueueV5<T>) >= 3 * kCacheLineSize, "");
    };

    ~QueueV5() {
        while (front())pop();
        // cout << "QueueV5 Deallocate" << endl;
        std::allocator_traits<Allocator>::deallocate(allocator_, buffer_, capacity_);
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
