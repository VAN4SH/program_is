#include "taskflow/event_journal.hpp"
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/task_service.hpp"
#include <iostream>
#include <memory>

int main() {
    using namespace taskflow;
    auto repo = std::make_shared<InMemoryTaskRepository>();
    auto priority = std::make_shared<ClosedLastPriorityStrategy>();
    TaskService service(repo, priority);

    EventJournal journal;
    service.subscribe([&](const Task& task) { journal.on_task_created(task); });

    service.create_task("Prepare requirements", "Analyst", 2, "Собрать требования");
    service.create_task("Implement repository", "Developer", 1, "InMemory + stub SQLite");
    service.create_task("Write tests", "QA", 3, "Unit + integration");

    std::cout << "Tasks by priority:\n";
    for (const auto& task : service.list_by_priority()) {
        std::cout << task.id << " | " << task.title << " | "
                  << task.assignee << " | " << to_string(task.status)
                  << " | priority=" << task.priority
                  << " | desc=" << task.description << '\n';
    }

    std::cout << "\nEvents in journal: " << journal.size() << '\n';
    for (const auto& event : journal.events()) {
        std::cout << event.name << " id=" << event.task_id
                  << " details=" << event.details << '\n';
    }
    return 0;
}
