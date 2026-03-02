#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "Types.h"
#include <mutex>
#include <queue>
#include <optional>
#include <condition_variable>

namespace ms {

/// 线程安全的帧队列
/// 用于在取流/解码线程与消费线程之间传递 YuvFrame
class FrameQueue {
public:
    explicit FrameQueue(size_t maxSize = 3)
        : maxSize_(maxSize) {}

    /// 压入一帧，如果队列满则丢弃最旧的帧
    void push(YuvFrame frame) {
        std::lock_guard<std::mutex> lock(mutex_);
        while (queue_.size() >= maxSize_) {
            queue_.pop();  // 丢弃旧帧，保持低延迟
        }
        queue_.push(std::move(frame));
        cv_.notify_one();
    }

    /// 尝试弹出一帧（非阻塞）
    std::optional<YuvFrame> tryPop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        YuvFrame frame = std::move(queue_.front());
        queue_.pop();
        return frame;
    }

    /// 阻塞弹出一帧，带超时
    std::optional<YuvFrame> waitPop(int timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                          [this] { return !queue_.empty(); })) {
            return std::nullopt;
        }
        YuvFrame frame = std::move(queue_.front());
        queue_.pop();
        return frame;
    }

    /// 清空队列
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) queue_.pop();
    }

    /// 当前队列大小
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    mutable std::mutex      mutex_;
    std::condition_variable cv_;
    std::queue<YuvFrame>    queue_;
    size_t                  maxSize_;
};

} // namespace ms

