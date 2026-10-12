// ЛР4 индивидуальное: смешанный GET + POST
import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  vus: 20,
  duration: '30s',
  thresholds: {
    http_req_failed: ['rate<0.01'],
    http_req_duration: ['p(95)<300'],
  },
};

export default function () {
  if (__ITER % 2 === 0) {
    const payload = JSON.stringify({
      title: `mixed-${__VU}-${__ITER}`,
      assignee: 'student',
      priority: 2,
    });
    const params = { headers: { 'Content-Type': 'application/json' } };
    const created = http.post('http://localhost:8080/tasks', payload, params);
    check(created, { 'created status is 201': (r) => r.status === 201 });
  } else {
    const listed = http.get('http://localhost:8080/tasks');
    check(listed, { 'list status is 200': (r) => r.status === 200 });
  }
  sleep(0.5);
}
