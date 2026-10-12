#pragma once
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/sqlite_task_repository.hpp"
#include <memory>
#include <stdexcept>
#include <string>

namespace taskflow {

struct Config {
    std::string storage{"memory"};
    std::string db_path{"taskflow.db"};
};

inline TaskRepositoryPtr make_repository(const Config& config) {
    if (config.storage == "memory") {
        return std::make_shared<InMemoryTaskRepository>();
    }
    if (config.storage == "sqlite") {
        // ЛР2: фабрика умеет создавать SQLite-адаптер
        return std::make_shared<SQLiteTaskRepository>(config.db_path);
    }
    throw std::invalid_argument("Unknown storage mode: " + config.storage);
}

} // namespace taskflow
