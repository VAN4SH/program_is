import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  vus: 20,
  duration: '1m',
  thresholds: {
    http_req_failed: ['rate<0.01'],
    http_req_duration: ['p(95)<300'],
  },
};

export default function () {
  const payload = JSON.stringify({
    title: `task-${__VU}-${__ITER}`,
    assignee: 'student',
    priority: 3,
  });
  const params = { headers: { 'Content-Type': 'application/json' } };
  const created = http.post('http://localhost:8080/tasks', payload, params);
  check(created, { 'created status is 201': (r) => r.status === 201 });
  const listed = http.get('http://localhost:8080/tasks');
  check(listed, { 'list status is 200': (r) => r.status === 200 });
  sleep(1);
}
