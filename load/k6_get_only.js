// ЛР4 индивидуальное: сценарий только GET /tasks
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
  const listed = http.get('http://localhost:8080/tasks');
  check(listed, { 'list status is 200': (r) => r.status === 200 });
  sleep(0.5);
}
