#include "taskflow/priority_strategy.hpp"

namespace taskflow {

int DefaultPriorityStrategy::score(const Task& task) const {
    return 10 - task.priority;
}

int ClosedLastPriorityStrategy::score(const Task& task) const {
    return is_closed(task.status) ? 1000 : 10 - task.priority;
}

int AssigneePriorityStrategy::score(const Task& task) const {
    // Чем меньше число, тем выше в списке.
    // Без исполнителя — в конец.
    if (task.assignee.empty()) {
        return 500;
    }
    return 10 - task.priority;
}

} // namespace taskflow
