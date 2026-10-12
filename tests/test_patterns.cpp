#include "simple_test.hpp"
#include "taskflow/event_journal.hpp"
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/repository_factory.hpp"
#include "taskflow/sqlite_task_repository.hpp"
#include "taskflow/task_service.hpp"
#include <memory>

using namespace taskflow;

TEST_CASE("factory creates in-memory repository") {
    Config config;
    config.storage = "memory";
    auto repo = make_repository(config);
    auto task = repo->create(Task{0, "Factory task", "Dev", TaskStatus::New, 3, ""});
    CHECK_TRUE(repo->find_by_id(task.id).has_value());
}

TEST_CASE("priority strategy puts closed tasks last") {
    auto repo = std::make_shared<InMemoryTaskRepository>();
    auto strategy = std::make_shared<ClosedLastPriorityStrategy>();
    TaskService service(repo, strategy);
    auto high = service.create_task("High", "A", 1);
    auto low = service.create_task("Low", "B", 5);
    service.transition(high.id, TaskStatus::InProgress);
    service.transition(high.id, TaskStatus::Done);
    auto ordered = service.list_by_priority();
    CHECK_EQ(ordered.front().id, low.id);
}

// --- ЛР2: новые проверки паттернов ---
TEST_CASE("factory creates sqlite repository stub") {
    Config config;
    config.storage = "sqlite";
    config.db_path = "tmp_lab2.db";
    auto repo = make_repository(config);
    auto* sqlite = dynamic_cast<SQLiteTaskRepository*>(repo.get());
    CHECK_TRUE(sqlite != nullptr);
    CHECK_EQ(sqlite->db_path(), std::string("tmp_lab2.db"));
}

TEST_CASE("factory rejects unknown storage") {
    Config config;
    config.storage = "oracle";
    CHECK_THROWS(make_repository(config));
}

TEST_CASE("assignee priority strategy puts empty assignee last") {
    auto repo = std::make_shared<InMemoryTaskRepository>();
    auto strategy = std::make_shared<AssigneePriorityStrategy>();
    // Создаём задачу с исполнителем через сервис, вторую правим через repo
    TaskService service(repo, strategy);
    auto with_user = service.create_task("With user", "Ivan", 3);
    Task empty{0, "No user", "tmp", TaskStatus::New, 1, ""};
    empty = repo->create(empty);
    empty.assignee = "";
    repo->update(empty);
    auto ordered = service.list_by_priority();
    CHECK_EQ(ordered.front().id, with_user.id);
    CHECK_EQ(ordered.back().id, empty.id);
}

TEST_CASE("observer writes TaskCreated to journal") {
    auto service = TaskService(std::make_shared<InMemoryTaskRepository>(),
                               std::make_shared<DefaultPriorityStrategy>());
    EventJournal journal;
    service.subscribe([&](const Task& task) { journal.on_task_created(task); });
    auto task = service.create_task("Observed", "Ivan", 2, "hello");
    CHECK_TRUE(journal.size() == 1);
    CHECK_EQ(journal.events().front().name, std::string("TaskCreated"));
    CHECK_EQ(journal.events().front().task_id, task.id);
}

TEST_CASE("sqlite stub throws on create") {
    SQLiteTaskRepository repo("demo.db");
    CHECK_THROWS(repo.create(Task{0, "x", "y", TaskStatus::New, 1, ""}));
}
