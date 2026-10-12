#include "simple_test.hpp"
#include "taskflow/event_journal.hpp"
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/task_service.hpp"
#include <memory>
#include <string>

using namespace taskflow;

static TaskService make_service() {
    return TaskService(std::make_shared<InMemoryTaskRepository>(),
                       std::make_shared<DefaultPriorityStrategy>());
}

TEST_CASE("create_task stores a valid task") {
    auto service = make_service();
    auto task = service.create_task("Prepare report", "Ivan", 2);
    CHECK_TRUE(task.id > 0);
    CHECK_EQ(task.status, TaskStatus::New);
    CHECK_EQ(service.find(task.id)->title, std::string("Prepare report"));
}

TEST_CASE("create_task rejects invalid priority") {
    auto service = make_service();
    CHECK_THROWS(service.create_task("Bad task", "Ivan", 7));
}

TEST_CASE("transition follows allowed workflow") {
    auto service = make_service();
    auto task = service.create_task("Implement service", "Maria", 1);
    CHECK_TRUE(service.transition(task.id, TaskStatus::InProgress));
    CHECK_TRUE(service.transition(task.id, TaskStatus::Done));
    CHECK_TRUE(!service.transition(task.id, TaskStatus::InProgress));
}

// --- ЛР1: индивидуальное задание ---
TEST_CASE("assign updates assignee for open task") {
    auto service = make_service();
    auto task = service.create_task("Fix bug", "Ivan", 2);
    CHECK_TRUE(service.assign(task.id, "Maria"));
    CHECK_EQ(service.find(task.id)->assignee, std::string("Maria"));
}

TEST_CASE("assign rejects done task") {
    auto service = make_service();
    auto task = service.create_task("Close me", "Ivan", 2);
    service.transition(task.id, TaskStatus::InProgress);
    service.transition(task.id, TaskStatus::Done);
    CHECK_TRUE(!service.assign(task.id, "Maria"));
}

TEST_CASE("description longer than 1000 is rejected") {
    auto service = make_service();
    const std::string long_desc(1001, 'x');
    CHECK_THROWS(service.create_task("Title", "Ivan", 2, long_desc));
}

TEST_CASE("description up to 1000 is accepted") {
    auto service = make_service();
    const std::string ok_desc(1000, 'a');
    auto task = service.create_task("Title", "Ivan", 2, ok_desc);
    CHECK_EQ(task.description.size(), static_cast<std::size_t>(1000));
}

// --- ЛР3: negative / unit / integration ---
TEST_CASE("create_task rejects empty title") {
    auto service = make_service();
    CHECK_THROWS(service.create_task("", "Ivan", 2));
}

TEST_CASE("create_task rejects empty assignee") {
    auto service = make_service();
    CHECK_THROWS(service.create_task("Title", "", 2));
}

TEST_CASE("create_task rejects priority below range") {
    auto service = make_service();
    CHECK_THROWS(service.create_task("Title", "Ivan", 0));
}

TEST_CASE("create_task rejects priority above range") {
    auto service = make_service();
    CHECK_THROWS(service.create_task("Title", "Ivan", 6));
}

TEST_CASE("transition New to Cancelled is allowed") {
    auto service = make_service();
    auto task = service.create_task("Cancel me", "Ivan", 3);
    CHECK_TRUE(service.transition(task.id, TaskStatus::Cancelled));
    CHECK_EQ(service.find(task.id)->status, TaskStatus::Cancelled);
}

TEST_CASE("transition Done to InProgress is forbidden") {
    auto service = make_service();
    auto task = service.create_task("Done task", "Ivan", 3);
    service.transition(task.id, TaskStatus::InProgress);
    service.transition(task.id, TaskStatus::Done);
    CHECK_TRUE(!service.transition(task.id, TaskStatus::InProgress));
}

TEST_CASE("assign rejects empty assignee") {
    auto service = make_service();
    auto task = service.create_task("Task", "Ivan", 3);
    CHECK_THROWS(service.assign(task.id, ""));
}

TEST_CASE("find returns nullopt for unknown id") {
    auto service = make_service();
    CHECK_TRUE(!service.find(999).has_value());
}

TEST_CASE("integration repository create find update list") {
    auto repo = std::make_shared<InMemoryTaskRepository>();
    Task draft{0, "Repo task", "Dev", TaskStatus::New, 2, "desc"};
    auto created = repo->create(draft);
    CHECK_TRUE(created.id > 0);
    CHECK_TRUE(repo->find_by_id(created.id).has_value());
    created.status = TaskStatus::InProgress;
    CHECK_TRUE(repo->update(created));
    auto all = repo->list();
    CHECK_TRUE(all.size() == 1);
    CHECK_EQ(all.front().status, TaskStatus::InProgress);
}

TEST_CASE("list_by_priority orders by priority score") {
    // DefaultPriorityStrategy: score = 10 - priority, сортировка по возрастанию score
    auto service = TaskService(std::make_shared<InMemoryTaskRepository>(),
                               std::make_shared<DefaultPriorityStrategy>());
    auto low = service.create_task("Low", "A", 5);   // score=5
    auto high = service.create_task("High", "B", 1); // score=9
    auto ordered = service.list_by_priority();
    CHECK_EQ(ordered.front().id, low.id);
    CHECK_EQ(ordered.back().id, high.id);
}
