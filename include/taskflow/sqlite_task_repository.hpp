#pragma once
#include "taskflow/repository.hpp"
#include <stdexcept>
#include <string>

namespace taskflow {

// ЛР2: заглушка SQLite-адаптера (без реальной БД, чтобы проект собирался
// даже без установленного libsqlite3). Методы бросают исключение.
class SQLiteTaskRepository final : public ITaskRepository {
public:
    explicit SQLiteTaskRepository(std::string db_path);

    Task create(Task draft) override;
    std::optional<Task> find_by_id(std::int64_t id) const override;
    std::vector<Task> list() const override;
    bool update(const Task& task) override;

    const std::string& db_path() const { return db_path_; }

private:
    std::string db_path_;
    [[noreturn]] void not_implemented() const;
};

} // namespace taskflow
