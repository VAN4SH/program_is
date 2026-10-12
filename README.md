# TaskFlow C++ учебная кодовая база

Сквозной проект для 4 лабораторных работ курса «Программирование информационных систем».

## Сборка (WSL / Linux)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_HTTP_SERVER=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/taskflow_cli
./build/taskflow_server   # HTTP API на порту 8080
```

## Связь с лабораторными

- ЛР 1: доменная модель, `description`, `assign()`, сервис, in-memory repository, CMake, CTest.
- ЛР 2: Strategy, Factory, заглушка SQLite adapter, Observer (`EventJournal`).
- ЛР 3: расширенные автотесты, negative tests, CI, sanitizers.
- ЛР 4: HTTP API (`/health`, `/tasks`), Docker, k6 load tests.

## HTTP API (ЛР 4)

- `GET /health` → `{"status":"ok"}`
- `GET /tasks` → список задач
- `POST /tasks` → тело `{"title":"...","assignee":"...","priority":3}`

## Docker

```bash
docker compose -f docker/docker-compose.yml up --build
curl http://localhost:8080/health
```

## Нагрузочные сценарии

```bash
k6 run load/k6_taskflow.js
k6 run load/k6_get_only.js
k6 run load/k6_mixed.js
```
