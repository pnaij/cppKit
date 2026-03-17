//
// Created by jianp on 2026/3/17.
//

#include "mpmc_bounded_queue.h"

#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

// ====================== 测试工具 ======================
std::mutex print_mutex;
#define LOG(msg) {\
    std::lock_guard<std::mutex> lock(print_mutex);\
    std::cout << "[测试] " << msg << std::endl;\
}

// ====================== 测试1：单线程 ======================
void test_single_thread() {
    LOG("===== 单线程基础测试 开始 =====");
    mpmc_bounded_queue<int> q(4);

    assert(q.enqueue(1));
    assert(q.enqueue(2));
    assert(q.enqueue(3));
    assert(q.enqueue(4));
    assert(!q.enqueue(5));

    int v;
    assert(q.dequeue(v) && v == 1);
    assert(q.dequeue(v) && v == 2);
    assert(q.dequeue(v) && v == 3);
    assert(q.dequeue(v) && v == 4);
    assert(!q.dequeue(v));

    assert(q.enqueue(10));
    assert(q.dequeue(v) && v == 10);
    LOG("===== 单线程基础测试 全部通过 =====\n");
}

// ====================== 测试2：边界空/满 ======================
void test_boundary() {
    LOG("===== 边界测试（空/满）开始 =====");
    mpmc_bounded_queue<int> q(2);
    int v;

    assert(!q.dequeue(v));
    assert(q.enqueue(1));
    assert(q.enqueue(2));
    assert(!q.enqueue(3));
    assert(q.dequeue(v));
    assert(q.dequeue(v));
    assert(!q.dequeue(v));
    LOG("===== 边界测试 全部通过 =====\n");
}

// ====================== 测试3：单生产者单消费者 ======================
void test_spsc() {
    LOG("===== 单生产者单消费者 测试 开始 =====");
    const int N = 100000;
    mpmc_bounded_queue<int> q(1024);

    std::thread producer([&]() {
        for (int i = 1; i <= N; ++i) {
            while (!q.enqueue(i));
        }
    });

    std::thread consumer([&]() {
        int v, last = 0;
        for (int i = 0; i < N; ++i) {
            while (!q.dequeue(v));
            assert(v == last + 1);
            last = v;
        }
    });

    producer.join();
    consumer.join();
    LOG("===== 单生产者单消费者 测试 全部通过 =====\n");
}

// ====================== 测试4：多生产者多消费者（压力测试） ======================
void test_mpmc() {
    LOG("===== 多生产者多消费者 压力测试 开始 =====");

    const size_t PRODUCER = 4;
    const size_t CONSUMER = 4;
    const size_t COUNT = 50000;
    const size_t TOTAL = PRODUCER * COUNT;

    mpmc_bounded_queue<uint64_t> q(1024);
    std::atomic<uint64_t> sum{0};
    std::atomic<size_t> recv{0};

    std::vector<std::thread> pro_threads;
    for (size_t i = 0; i < PRODUCER; ++i) {
        pro_threads.emplace_back([&, i]() {
            uint64_t base = i * 100000000;
            for (size_t j = 0; j < COUNT; ++j) {
                while (!q.enqueue(base + j));
            }
        });
    }

    std::vector<std::thread> con_threads;
    for (size_t i = 0; i < CONSUMER; ++i) {
        con_threads.emplace_back([&]() {
            uint64_t v;
            while (recv.load() < TOTAL) {
                if (q.dequeue(v)) {
                    sum.fetch_add(v);
                    recv.fetch_add(1);
                }
            }
        });
    }

    for (auto& t : pro_threads) t.join();
    for (auto& t : con_threads) t.join();

    LOG("总接收数量: " + std::to_string(recv.load()) + "/" + std::to_string(TOTAL));
    assert(recv == TOTAL);
    LOG("===== 多生产者多消费者 测试 全部通过 =====\n");
}

// ====================== 主函数 ======================
int main() {
    try {
        test_single_thread();
        test_boundary();
        test_spsc();
        test_mpmc();

        LOG("\n=====================================");
        LOG("🎉 所有测试全部通过！");
        LOG("你的无锁队列实现完全正确！");
        LOG("=====================================\n");
        return 0;
    } catch (...) {
        LOG("❌ 测试失败！存在bug");
        return -1;
    }
}