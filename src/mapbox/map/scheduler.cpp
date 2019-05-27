#include <mapbox/map/scheduler.hpp>

#include <cassert>

namespace mapbox {
namespace map {

Scheduler::Scheduler() {
    assert(threadId_ == std::this_thread::get_id());
}

Scheduler::~Scheduler() {
    assert(threadId_ == std::this_thread::get_id());
}

void Scheduler::schedule(std::weak_ptr<mbgl::Mailbox> mailbox) {
    std::lock_guard<std::mutex> lock(taskQueueMutex_);
    taskQueue_.push(mailbox);

    for (auto it = notifyFns_.begin(); it != notifyFns_.end();) {
        auto fn = it->lock();
        if (fn) {
            fn->operator()();
            it++;
        } else {
            it = notifyFns_.erase(it);
        }
    }
}

void Scheduler::attach(std::weak_ptr<NotifyFn>&& fn) {
    notifyFns_.push_back(fn);
}

void Scheduler::processEvents() {
    assert(threadId_ == std::this_thread::get_id());

    std::queue<std::weak_ptr<mbgl::Mailbox>> taskQueue;
    {
        std::unique_lock<std::mutex> lock(taskQueueMutex_);
        std::swap(taskQueue, taskQueue_);
    }

    while (!taskQueue.empty()) {
        mbgl::Mailbox::maybeReceive(taskQueue.front());
        taskQueue.pop();
    }
}

}  // namespace map
}  // namespace mapbox
