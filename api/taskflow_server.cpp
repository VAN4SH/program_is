// Простой учебный HTTP-сервер для ЛР 4 (без сторонних библиотек).
// Поддерживает: GET /health, GET /tasks, POST /tasks
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/task_service.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string json_escape(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        if (c == '"' || c == '\\') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

std::string task_to_json(const taskflow::Task& task) {
    std::ostringstream oss;
    oss << "{"
        << "\"id\":" << task.id << ","
        << "\"title\":\"" << json_escape(task.title) << "\","
        << "\"assignee\":\"" << json_escape(task.assignee) << "\","
        << "\"status\":\"" << taskflow::to_string(task.status) << "\","
        << "\"priority\":" << task.priority << ","
        << "\"description\":\"" << json_escape(task.description) << "\""
        << "}";
    return oss.str();
}

std::string tasks_to_json(const std::vector<taskflow::Task>& tasks) {
    std::ostringstream oss;
    oss << "[";
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        if (i > 0) oss << ",";
        oss << task_to_json(tasks[i]);
    }
    oss << "]";
    return oss.str();
}

// Очень простой разбор JSON вида {"title":"...","assignee":"...","priority":3}
std::string extract_string_field(const std::string& body, const std::string& key) {
    const std::string pattern = "\"" + key + "\"";
    auto pos = body.find(pattern);
    if (pos == std::string::npos) return "";
    pos = body.find(':', pos);
    if (pos == std::string::npos) return "";
    pos = body.find('"', pos);
    if (pos == std::string::npos) return "";
    auto end = body.find('"', pos + 1);
    if (end == std::string::npos) return "";
    return body.substr(pos + 1, end - pos - 1);
}

int extract_int_field(const std::string& body, const std::string& key, int fallback) {
    const std::string pattern = "\"" + key + "\"";
    auto pos = body.find(pattern);
    if (pos == std::string::npos) return fallback;
    pos = body.find(':', pos);
    if (pos == std::string::npos) return fallback;
    try {
        return std::stoi(body.substr(pos + 1));
    } catch (...) {
        return fallback;
    }
}

std::string http_response(int code, const std::string& status, const std::string& body,
                          const std::string& content_type = "application/json") {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << code << " " << status << "\r\n"
        << "Content-Type: " << content_type << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    return oss.str();
}

} // namespace

int main() {
    using namespace taskflow;

    const int port = [] {
        if (const char* env = std::getenv("PORT")) {
            try { return std::stoi(env); } catch (...) {}
        }
        return 8080;
    }();

    auto repo = std::make_shared<InMemoryTaskRepository>();
    auto priority = std::make_shared<ClosedLastPriorityStrategy>();
    TaskService service(repo, priority);
    service.create_task("Demo task", "student", 3, "seed");

    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "socket failed\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind failed on port " << port << '\n';
        close(server_fd);
        return 1;
    }
    if (listen(server_fd, 16) < 0) {
        std::cerr << "listen failed\n";
        close(server_fd);
        return 1;
    }

    std::cout << "TaskFlow HTTP server listening on port " << port << '\n';

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) continue;

        // Читаем запрос целиком (учёт Content-Length для POST)
        std::string request;
        char chunk[2048];
        while (true) {
            const ssize_t n = recv(client, chunk, sizeof(chunk), 0);
            if (n <= 0) break;
            request.append(chunk, static_cast<std::size_t>(n));
            const auto header_end = request.find("\r\n\r\n");
            if (header_end == std::string::npos) continue;
            std::size_t content_length = 0;
            const auto cl_pos = request.find("Content-Length:");
            if (cl_pos != std::string::npos && cl_pos < header_end) {
                try {
                    content_length = static_cast<std::size_t>(
                        std::stoul(request.substr(cl_pos + 15)));
                } catch (...) {
                    content_length = 0;
                }
            }
            const std::size_t body_have = request.size() - (header_end + 4);
            if (body_have >= content_length) break;
        }
        if (request.empty()) {
            close(client);
            continue;
        }

        std::string method;
        std::string path;
        {
            std::istringstream first(request);
            first >> method >> path;
        }

        std::string response;
        if (method == "GET" && path == "/health") {
            response = http_response(200, "OK", "{\"status\":\"ok\"}");
        } else if (method == "GET" && (path == "/tasks" || path.rfind("/tasks?", 0) == 0)) {
            response = http_response(200, "OK", tasks_to_json(service.list_by_priority()));
        } else if (method == "POST" && path == "/tasks") {
            auto body_pos = request.find("\r\n\r\n");
            std::string body = body_pos == std::string::npos ? "" : request.substr(body_pos + 4);
            const std::string title = extract_string_field(body, "title");
            const std::string assignee = extract_string_field(body, "assignee");
            const int priority_value = extract_int_field(body, "priority", 3);
            try {
                auto task = service.create_task(title, assignee, priority_value);
                response = http_response(201, "Created", task_to_json(task));
            } catch (const std::exception& ex) {
                response = http_response(400, "Bad Request",
                                         std::string("{\"error\":\"") + json_escape(ex.what()) + "\"}");
            }
        } else {
            response = http_response(404, "Not Found", "{\"error\":\"not found\"}");
        }

        send(client, response.data(), response.size(), 0);
        close(client);
    }

    close(server_fd);
    return 0;
}
