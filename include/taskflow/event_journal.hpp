#pragma once
#include "taskflow/task.hpp"
#include <string>
#include <vector>

namespace taskflow {

// ЛР2 (индивидуальное): простой журнал событий Observer
struct DomainEvent {
    std::string name;
    std::int64_t task_id{};
    std::string details;
};

class EventJournal {
public:
    void on_task_created(const Task& task) {
        events_.push_back(DomainEvent{"TaskCreated", task.id, task.title});
    }

    const std::vector<DomainEvent>& events() const { return events_; }
    std::size_t size() const { return events_.size(); }
    void clear() { events_.clear(); }

private:
    std::vector<DomainEvent> events_;
};

} // namespace taskflow
