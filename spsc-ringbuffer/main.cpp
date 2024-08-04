#include <chrono>

#include <cassert>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <algorithm> 

// C++20
#include <utility>
#include <concepts>

#include <unistd.h>

#include <sys/mman.h> // mmap, munmap


#include "queue_1_lock.h"
#include "queue_2_atomic_cst.h"
#include "queue_3_atomic_acqrel.h"
#include "queue_4_cache_fs_p2.h"
#include "queue_5_hugetlb.h"

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

template<typename T>
// requires std::is_base_of<QueueBase<T::InnerType>, T>::value
double queue_benchmark() {
    T q(QUEUE_SIZE);

    jthread producer([&]() {
        pinThread(0);
        for (uint64_t i = 0; i < ITERATIONS; i++) {
            while (!q.emplace(i)) {
                // busy wait
            }
            // this_thread::sleep_for(chrono::microseconds(1));
        }
        return 0;
    });

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
        while(q.empty()) {
            // busy wait
        }
        q.pop();
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
        uint64_t val = *q1.front();
        q2.emplace(val);
        q1.pop();
      }
    });

    pinThread(1);

    auto start = std::chrono::steady_clock::now();
    for (uint64_t i=0; i < ITERATIONS; ++i) {
      q1.emplace(i);
      while (!q2.front())
        ;
      q2.pop();
    }
    auto end = std::chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start);

    double rtt_ns = (double)duration.count() / (double)ITERATIONS;
    return rtt_ns;
}

template <typename T> struct HugePageAllocator {
  using value_type = T;

  struct AllocationResult {
    T *ptr;
    size_t count;
  };

  size_t roundup(size_t n) { return (((n - 1) >> 21) + 1) << 21; }

  AllocationResult allocate_at_least(size_t n) {
    size_t count = roundup(sizeof(T) * n);
    auto p = static_cast<T *>(mmap(nullptr, count, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0));
    if (p == MAP_FAILED) {
      throw std::bad_alloc();
    }
    return {p, count / sizeof(T)};
  }

  void deallocate(T *p, size_t n) { munmap(p, roundup(sizeof(T) * n)); }
};

#include <locale>
#include <memory>

struct separate_thousands : std::numpunct<char> {
    char_type do_thousands_sep() const override { return '\''; }  // separate with commas
    string_type do_grouping() const override { return "\3"; } // groups of 3 digit
};


int main(int argc, char** argv) {
    // get first arg as number
    int queue_v = -1;
    if (argc > 1) {
        queue_v = std::stoul(argv[1]);
    }
    auto thousands = std::make_unique<separate_thousands>();
    std::cout.imbue(std::locale(std::cout.getloc(), thousands.release()));
    cout << "Iterations: " << ITERATIONS << endl;
    cout << "Size: " << QUEUE_SIZE << endl;
    cout << "Loops: " << ITERATIONS/QUEUE_SIZE << endl;
    
    int page_size_kb = getpagesize() / 1024;
    cout << "Page Size: " << page_size_kb << "K" << endl;
    cout << "Size / " << page_size_kb << "K: " << (QUEUE_SIZE * sizeof(uint64_t)) / 1024 / page_size_kb << endl;
    cout << "Huge Page Size: " << "2048K" << endl;
    cout << "Size / 2M: " << (QUEUE_SIZE * sizeof(uint64_t)) / 1024 / 1024 / 2 << endl;

    // list of queue types for each value of queue_v run that else run all if queue_v is -1
    if (queue_v == -1 || queue_v == 1) {
        double ops_ms, rtt_ns;
        printf("QueueV1... lock\n");
        ops_ms = queue_benchmark<QueueV1<uint64_t>>();
        rtt_ns = queue_latency<QueueV1<uint64_t>>();
        printf("QueueV1: %2.f ops/ms, RTT: %2.f ns\n", ops_ms, rtt_ns);
    }
    if (queue_v == -1 || queue_v == 2) {
        printf("QueueV2... atomic cst\n");
        double ops_ms, rtt_ns;
        ops_ms = queue_benchmark<QueueV2<uint64_t>>();
        rtt_ns = queue_latency<QueueV2<uint64_t>>();
        printf("QueueV2: %2.f ops/ms, RTT: %2.f ns\n", ops_ms, rtt_ns);
    }
    if (queue_v == -1 || queue_v == 3) {
        printf("QueueV3... atomic acqrel\n");
        double ops_ms, rtt_ns;
        ops_ms = queue_benchmark<QueueV3<uint64_t>>();
        rtt_ns = queue_latency<QueueV3<uint64_t>>();
        printf("QueueV3: %2.f ops/ms, RTT: %2.f ns\n", ops_ms, rtt_ns);
    }
    if (queue_v == -1 || queue_v == 4) {
        printf("QueueV4... cached r/w idx, cache line alignment, pow2 masks\n");
        double ops_ms, rtt_ns;
        ops_ms = queue_benchmark<QueueV4<uint64_t>>();
        rtt_ns = queue_latency<QueueV4<uint64_t>>();
        printf("QueueV4: %2.f ops/ms, RTT: %2.f ns\n", ops_ms, rtt_ns);
    }
    if (queue_v == -1 || queue_v == 5) {
        printf("QueueV5... hugepages\n");
        double ops_ms, rtt_ns;
        ops_ms = queue_benchmark<QueueV5<uint64_t, HugePageAllocator<uint64_t>>>();
        rtt_ns = queue_latency<QueueV5<uint64_t, HugePageAllocator<uint64_t>>>();
        printf("QueueV5: %2.f ops/ms, RTT: %2.f ns\n", ops_ms, rtt_ns);
    }
    return 0;
}