#pragma once
#include <stddef.h>

template<class T>
class QueueBase {
public:
    virtual ~QueueBase() = default;
    virtual bool enqueue(const T& data) = 0;
    virtual bool dequeue(T* data) = 0;
    
    virtual bool full() const = 0;
    virtual bool empty() const = 0;

    virtual T* front() = 0;

    virtual size_t capacity() const = 0;
};