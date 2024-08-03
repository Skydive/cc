#include <chrono>

#include "queue_1_lock.h"

#define ITERATIONS 1000000

using namespace std;

int producer(QueueBase<int>* q) {
    for (int i = 0; i < ITERATIONS; i++) {
        while (!q->enqueue(i)) {
            // busy wait
        }
    }
    return 0;
}

int consumer(QueueBase<int>* q) {
    int data;

    for (int i = 0; i < ITERATIONS; i++) {
        while (!q->dequeue(&data)) {
            // busy wait
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    size_t capacity = (ITERATIONS / 1000) + 1;

    QueueV1<int> q(capacity);
    auto start = chrono::system_clock::now();


    auto end = chrono::system_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

    assert(q.front() == nullptr);
    assert(sum == iter * (iter - 1) / 2);

    producer(&q);



}