#include <chrono>

#include "queue_1_lock.h"
#include <cassert>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <algorithm> 

// C++20
#include <utility>
#include <concepts>

#define ITERATIONS 1000000UL
#define QUEUE_SIZE 1000000UL
using namespace std;


void pinThread(int cpu) {
  if (cpu < 0) {
    return;
  }
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) ==
      -1) {
    perror("pthread_setaffinity_no");
    exit(1);
  }
}

int producer_thread(QueueBase<uint64_t>* q) {
    pinThread(0);
    for (uint64_t i = 0; i < ITERATIONS; i++) {
        while (!q->enqueue(i)) {
            // busy wait
        }
        // this_thread::sleep_for(chrono::microseconds(1));
    }
    return 0;
}

template<typename T>
// requires std::is_base_of<QueueBase<T::InnerType>, T>::value
double queue_benchmark() {
    T q(QUEUE_SIZE);

    jthread producer(producer_thread, &q);

    pinThread(1);
    auto start = chrono::steady_clock::now();

    // consumer and sum
    uint64_t sum = 0;
    for (uint64_t i = 0; i < ITERATIONS; i++) {
        uint64_t* data = nullptr;
        while(data == nullptr) {
            data = q.front();
        }
        sum += *data;
        while(!q.dequeue(data)) {
            // busy wait
        }
    }
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);


    // cout << "Sum " << sum << endl;
    // cout << "Expected " << ITERATIONS * (ITERATIONS - 1) / 2 << endl;
    assert(q.front() == nullptr);
    assert(sum == ITERATIONS * (ITERATIONS - 1) / 2);
    double ops_ms = (double)ITERATIONS/(double)duration.count();
    return ops_ms;
}

template<typename T>
double queue_latency() {
    T q1(QUEUE_SIZE), q2(QUEUE_SIZE);
    auto t = jthread([&] {
      pinThread(0);
      for (uint64_t i = 0; i < ITERATIONS; ++i) {
        while (!q1.front())
          ;
        q2.enqueue(*q1.front());
        q1.pop();
      }
    });

    pinThread(1);

    auto start = std::chrono::steady_clock::now();
    for (uint64_t i=0; i < ITERATIONS; ++i) {
      q1.enqueue(i);
      while (!q2.front())
        ;
      q2.pop();
    }
    auto end = std::chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start);

    double rtt_ns = (double)duration.count() / (double)ITERATIONS;
    return rtt_ns;
}

int main(int argc, char** argv) {
    double ops_ms, rtt_ns;
    ops_ms = queue_benchmark<QueueV1<uint64_t>>();
    rtt_ns = queue_latency<QueueV1<uint64_t>>();

    printf("QueueV1: %2.f ops/ms, RTT: %2.f ns\n", ops_ms, rtt_ns);

    return 0;
}