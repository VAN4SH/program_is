#include "taskflow/task_service.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace taskflow {

std::string to_string(TaskStatus status) {
    switch (status) {
        case TaskStatus::New: return "New";
        case TaskStatus::InProgress: return "InProgress";
        case TaskStatus::Done: return "Done";
        case TaskStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

TaskStatus status_from_string(const std::string& value) {
    if (value == "New") return TaskStatus::New;
    if (value == "InProgress") return TaskStatus::InProgress;
    if (value == "Done") return TaskStatus::Done;
    if (value == "Cancelled") return TaskStatus::Cancelled;
    throw std::invalid_argument("Unknown task status: " + value);
}

TaskService::TaskService(TaskRepositoryPtr repository, PriorityStrategyPtr priority_strategy)
    : repository_(std::move(repository)), priority_strategy_(std::move(priority_strategy)) {
    if (!repository_ || !priority_strategy_) {
        throw std::invalid_argument("TaskService dependencies must not be null");
    }
}

void TaskService::subscribe(TaskCreatedHandler handler) {
    if (handler) {
        observers_.push_back(std::move(handler));
    }
}

Task TaskService::create_task(std::string title, std::string assignee, int priority,
                              std::string description) {
    validate_task_input(title, assignee, priority, description);
    Task draft{0, std::move(title), std::move(assignee), TaskStatus::New, priority,
               std::move(description)};
    auto created = repository_->create(std::move(draft));
    // ЛР2: уведомляем подписчиков Observer
    for (const auto& handler : observers_) {
        handler(created);
    }
    return created;
}

bool TaskService::assign(std::int64_t id, std::string assignee) {
    if (assignee.empty()) {
        throw std::invalid_argument("assignee must not be empty");
    }
    auto task = repository_->find_by_id(id);
    if (!task || is_closed(task->status)) {
        return false;
    }
    task->assignee = std::move(assignee);
    return repository_->update(*task);
}

bool TaskService::transition(std::int64_t id, TaskStatus next_status) {
    auto task = repository_->find_by_id(id);
    if (!task) {
        return false;
    }
    if (!can_transition(task->status, next_status)) {
        return false;
    }
    task->status = next_status;
    return repository_->update(*task);
}

std::vector<Task> TaskService::list_by_priority() const {
    auto tasks = repository_->list();
    std::sort(tasks.begin(), tasks.end(), [&](const Task& a, const Task& b) {
        const int left = priority_strategy_->score(a);
        const int right = priority_strategy_->score(b);
        if (left == right) return a.id < b.id;
        return left < right;
    });
    return tasks;
}

std::optional<Task> TaskService::find(std::int64_t id) const {
    return repository_->find_by_id(id);
}

void TaskService::validate_task_input(const std::string& title, const std::string& assignee,
                                      int priority, const std::string& description) const {
    if (title.empty()) {
        throw std::invalid_argument("title must not be empty");
    }
    if (assignee.empty()) {
        throw std::invalid_argument("assignee must not be empty");
    }
    if (priority < 1 || priority > 5) {
        throw std::invalid_argument("priority must be in [1;5]");
    }
    // ЛР1: описание не длиннее 1000 символов
    if (description.size() > 1000) {
        throw std::invalid_argument("description must be at most 1000 characters");
    }
}

bool TaskService::can_transition(TaskStatus current, TaskStatus next) const {
    if (current == next) return true;
    if (is_closed(current)) return false;
    if (current == TaskStatus::New && (next == TaskStatus::InProgress || next == TaskStatus::Cancelled)) return true;
    if (current == TaskStatus::InProgress && (next == TaskStatus::Done || next == TaskStatus::Cancelled)) return true;
    return false;
}

} // namespace taskflow
