#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>
#include "ThreadPool.h"

std::mutex printMutex;

template <typename T> void safePrint(T i) {
    std::lock_guard<std::mutex> lock(printMutex);
    std::cout << i << std::endl;
}


int main() {
    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for(int i = 0; i < 8; ++i) {
        auto future = pool.enqueue([i] {
            safePrint("hello " + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::seconds(1));
            safePrint("world " + std::to_string(i));
            return i*i;
        });
        if (future) {
            (*future).get();
            results.emplace_back(std::move(*future));
        }
    }

    for(auto && result: results) {
        int val = result.get();
        safePrint("result " + std::to_string(val));
    }
}
