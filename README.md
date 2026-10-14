# MyTinyWebServer

A Linux C++17 HTTP server using epoll, a worker pool, buffered I/O, and a MySQL connection pool. It serves files from `wwwroot/` and includes a small login demonstration.

## Build

On Ubuntu, install a C++ compiler, CMake, pkg-config, and the MySQL client development library:

```sh
sudo apt-get install build-essential cmake pkg-config default-libmysqlclient-dev
cmake -S . -B build
cmake --build build --parallel
```

Linux is required because the server uses epoll. The executable is `build/MyTinyWebServer`; static files resolve from the checkout regardless of the current working directory.

## Configure and run

```sh
export MYSQL_HOST=127.0.0.1
export MYSQL_USER=webserver
export MYSQL_PASSWORD='your-local-database-password'
export MYSQL_DATABASE=webserver
./build/MyTinyWebServer 9096
```

`PORT` defaults to 9096, `MYSQL_PORT` to 3306, and `MYSQL_POOL_SIZE` to 10. The optional command-line port overrides `PORT`. Invalid numeric settings and missing database credentials produce a startup error. The database must exist before startup.

For the login demo, create a `user` table with `username VARCHAR(50)` and `passwd VARCHAR(50)` columns. Login accepts URL-encoded `username` and `pwd` fields and queries by a bound username parameter. The educational demo stores plaintext passwords; replace this with password hashing and a session layer before using it for an application. A successful login serves a demonstration page; it does not establish an authenticated session.

Open http://localhost:9096/ or use:

```sh
curl -i http://localhost:9096/
curl -i http://localhost:9096/missing
```

## Implementation

- `include/`: epoll, thread pool, and server interfaces.
- `httpconnector/`: request parsing and response interfaces.
- `src/`: implementation and environment-based entry point.
- `buffer/`: dynamic read/write buffers.
- `sqlpool/`: database connection management.
- `log/`: asynchronous logging queue.
- `wwwroot/`: English demonstration and error pages.

Response file storage remains owned by its connection until transmission finishes. Writes handle interrupted and would-block operations without advancing buffers on failure. Form parsing validates percent escapes, and static paths reject traversal. Requests are buffered until their declared body is present and capped at 1 MiB.

## Development status

This is an educational HTTP/1.x implementation. It has no TLS, production session authentication, chunked request decoding, or complete HTTP pipelining support. Performance claims require a documented benchmark on a specific machine and workload. Use a reverse proxy and complete protocol/security review before broader deployment.

The repository does not currently contain an explicit license grant. MySQL client libraries and retained third-party headers keep their respective upstream licenses.
