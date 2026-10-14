"""Exercise real HTTP responses against a configured local test server."""
import http.client
import os
from pathlib import Path
import socket
import subprocess
import time

port = 19096
process = subprocess.Popen([str(Path('build/MyTinyWebServer').resolve()), str(port)], stdout=subprocess.DEVNULL)
try:
    for _ in range(100):
        if process.poll() is not None:
            raise RuntimeError('Server exited before accepting connections')
        try:
            with socket.create_connection(('127.0.0.1', port), timeout=0.2):
                break
        except OSError:
            time.sleep(0.1)
    else:
        raise RuntimeError('Server did not start within ten seconds')

    def request(path, body=None):
        connection = http.client.HTTPConnection('127.0.0.1', port, timeout=5)
        connection.request('POST' if body is not None else 'GET', path, body,
                           {'Content-Type': 'application/x-www-form-urlencoded'} if body is not None else {})
        response = connection.getresponse()
        payload = response.read().decode()
        assert int(response.getheader('Content-Length')) == len(payload.encode())
        connection.close()
        return response.status, payload

    assert request('/')[0] == 200
    large = Path('wwwroot/smoke-large.html')
    large.write_text('response-data-' * 20000)
    try:
        assert request('/smoke-large.html')[1] == large.read_text()
    finally:
        large.unlink()
    assert request('/missing')[0] == 404
    assert request('/../CMakeLists.txt')[0] == 400
    assert 'Signed in' in request('/login', 'username=demo&pwd=correct')[1]
    assert 'Sign-in failed' in request('/login', 'username=demo&pwd=wrong')[1]
    assert 'Sign-in failed' in request('/login', 'username=missing&pwd=correct')[1]
    assert request('/login', 'username=demo&pwd=%')[0] == 400
    # A fragmented body must not be parsed until every declared byte arrives.
    with socket.create_connection(('127.0.0.1', port), timeout=5) as sock:
        body = b'username=demo&pwd=correct'
        sock.sendall(b'POST /login HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: ' + str(len(body)).encode() + b'\r\n\r\n' + body[:4])
        time.sleep(0.05)
        sock.sendall(body[4:])
        response = http.client.HTTPResponse(sock)
        response.begin()
        assert b'Signed in' in response.read()
    print('HTTP smoke checks passed')
finally:
    process.terminate()
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait()
