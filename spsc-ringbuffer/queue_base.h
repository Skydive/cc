#pragma once
#include <stddef.h>

template<class T>
class QueueBase {
public:
    using InnerType = T;
    virtual ~QueueBase() = default;

    template <typename ...Args>
    bool emplace(Args&& ...args) noexcept; // Move semantics

    inline T* front() noexcept;

    inline void pop();

    inline bool full() const noexcept;
    inline bool empty() const noexcept;


    inline size_t capacity() const noexcept;
};