//
// Created by jianp on 2026/3/17.
//

#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <string>

// 测试函数1：无参数、无返回值，模拟耗时任务
void testTask1(int id) {
    std::cout << "任务1 - ID: " << id << " 开始执行，线程ID: " << std::this_thread::get_id() << std::endl;
    // 模拟耗时操作（100毫秒）
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "任务1 - ID: " << id << " 执行完成" << std::endl;
}

// 测试函数2：带参数、有返回值
int testTask2(int a, int b) {
    std::cout << "任务2 开始执行，计算: " << a << " + " << b << "，线程ID: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return a + b;
}

// 测试函数3：抛出异常
void testTask3() {
    std::cout << "任务3 开始执行，即将抛出异常，线程ID: " << std::this_thread::get_id() << std::endl;
    throw std::runtime_error("任务3主动抛出异常！");
}

int main() {
    try {
        // 1. 创建线程池，设置4个工作线程
        std::cout << "========== 创建线程池（4个线程）==========" << std::endl;
        ThreadPool pool(4);

        // 2. 测试：提交无返回值任务（批量提交5个）
        std::cout << "\n========== 测试无返回值任务 ==========" << std::endl;
        std::vector<std::future<void>> futures1;
        for (int i = 1; i <= 5; ++i) {
            futures1.emplace_back(pool.enqueue(testTask1, i));
        }
        // 等待所有无返回值任务执行完毕
        for (auto& f : futures1) {
            f.get();
        }

        // 3. 测试：提交带返回值任务
        std::cout << "\n========== 测试带返回值任务 ==========" << std::endl;
        auto future2 = pool.enqueue(testTask2, 10, 20);
        std::cout << "任务2计算结果: " << future2.get() << std::endl;

        // 4. 测试：任务内部抛出异常（线程池安全处理）
        std::cout << "\n========== 测试任务抛出异常 ==========" << std::endl;
        auto future3 = pool.enqueue(testTask3);
        try {
            future3.get();
        } catch (const std::exception& e) {
            std::cout << "捕获到任务异常: " << e.what() << std::endl;
        }

        // 5. 测试：Lambda表达式任务
        std::cout << "\n========== 测试Lambda任务 ==========" << std::endl;
        auto future4 = pool.enqueue([]() -> std::string {
            std::cout << "Lambda任务执行，线程ID: " << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            return "Lambda任务执行成功！";
        });
        std::cout << "Lambda返回结果: " << future4.get() << std::endl;

        // 6. 测试：线程池销毁后提交任务（触发异常）
        std::cout << "\n========== 测试线程池停止后提交任务 ==========" << std::endl;
        // 离开作用域，线程池自动析构
        {
            ThreadPool smallPool(1);
        }
        // 向已销毁的线程池提交任务，会抛出运行时异常
        pool.enqueue([](){});

    } catch (const std::exception& e) {
        std::cout << "主线程捕获异常: " << e.what() << std::endl;
    }

    std::cout << "\n========== 所有测试执行完毕 ==========" << std::endl;
    return 0;
}