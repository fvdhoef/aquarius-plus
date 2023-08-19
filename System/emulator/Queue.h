#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class Queue {
public:
    Queue() {
    }

    void push(const T &item) {
        std::lock_guard<std::mutex> guard(mutex);
        q.push(item);
        cv.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex);
        while (q.empty()) {
            cv.wait(lock);
        }
        auto value = q.front();
        q.pop();
        return value;
    }

    bool empty() {
        std::lock_guard<std::mutex> guard(mutex);
        return q.empty();
    }

private:
    std::queue<T>           q;
    std::mutex              mutex;
    std::condition_variable cv;
};
