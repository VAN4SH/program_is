#pragma once
#include "taskflow/priority_strategy.hpp"
#include "taskflow/repository.hpp"
#include <functional>
#include <string>
#include <vector>

namespace taskflow {

// ЛР2: простой Observer — подписчик на событие создания задачи
using TaskCreatedHandler = std::function<void(const Task&)>;

class TaskService {
public:
    TaskService(TaskRepositoryPtr repository, PriorityStrategyPtr priority_strategy);

    Task create_task(std::string title, std::string assignee, int priority,
                     std::string description = "");
    bool assign(std::int64_t id, std::string assignee);
    bool transition(std::int64_t id, TaskStatus next_status);
    std::vector<Task> list_by_priority() const;
    std::optional<Task> find(std::int64_t id) const;

    // ЛР2: подписка на событие TaskCreated
    void subscribe(TaskCreatedHandler handler);

private:
    void validate_task_input(const std::string& title, const std::string& assignee,
                             int priority, const std::string& description) const;
    bool can_transition(TaskStatus current, TaskStatus next) const;

    TaskRepositoryPtr repository_;
    PriorityStrategyPtr priority_strategy_;
    std::vector<TaskCreatedHandler> observers_;
};

} // namespace taskflow
