#include "taskflow/sqlite_task_repository.hpp"

namespace taskflow {

SQLiteTaskRepository::SQLiteTaskRepository(std::string db_path)
    : db_path_(std::move(db_path)) {}

void SQLiteTaskRepository::not_implemented() const {
    throw std::runtime_error(
        "SQLiteTaskRepository is a stub for Lab 2. Path: " + db_path_);
}

Task SQLiteTaskRepository::create(Task) {
    not_implemented();
}

std::optional<Task> SQLiteTaskRepository::find_by_id(std::int64_t) const {
    not_implemented();
}

std::vector<Task> SQLiteTaskRepository::list() const {
    not_implemented();
}

bool SQLiteTaskRepository::update(const Task&) {
    not_implemented();
}

} // namespace taskflow
