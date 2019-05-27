#pragma once

#include <mbgl/actor/mailbox.hpp>
#include <mbgl/actor/scheduler.hpp>

#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace mapbox {
namespace map {

class Scheduler : public mbgl::Scheduler {
public:
    using NotifyFn = std::function<void()>;

    explicit Scheduler();
    ~Scheduler() override;

    // mbgl::Scheduler implementation.
    void schedule(std::weak_ptr<mbgl::Mailbox>) final;
    void processEvents();

    void attach(std::weak_ptr<NotifyFn>&&);

private:
    const std::thread::id threadId_ = std::this_thread::get_id();

    std::list<std::weak_ptr<NotifyFn>> notifyFns_;

    std::mutex taskQueueMutex_;
    std::queue<std::weak_ptr<mbgl::Mailbox>> taskQueue_;
};

}  // namespace map
}  // namespace mapbox
