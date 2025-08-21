# MyTInyWebServer

A high-performance, concurrent HTTP web server built with modern C++17, featuring excellent concurrency handling capabilities and efficient resource management.

## 🚀 Features

- **High Concurrency**: Built with Epoll + ThreadPool architecture for superior concurrent performance
- **Modern C++**: Written in C++17 with modern programming practices
- **Event-Driven**: Non-blocking I/O with efficient event-driven programming
- **Database Integration**: Built-in MySQL connection pool support
- **Comprehensive Logging**: Full-featured logging system for debugging and monitoring
- **Buffer Management**: Efficient I/O buffer handling with dual-buffer design
- **HTTP Protocol**: Complete HTTP request/response processing
- **Cross-Platform**: Linux-based with potential for cross-platform expansion

## 🏗️ Architecture

### Core Components

- **WebServer**: Main server class implementing singleton pattern
- **Epoller**: High-performance event listener based on Linux epoll
- **ThreadPool**: Configurable worker thread pool for task execution
- **HttpConnector**: HTTP connection handler with request/response processing
- **Buffer**: Efficient I/O buffer management system
- **SQLPool**: MySQL database connection pool
- **Logger**: Comprehensive logging system

### Design Patterns

- **Singleton Pattern**: Ensures single WebServer instance
- **Event-Driven Architecture**: Efficient event handling with epoll
- **Producer-Consumer**: Thread pool with task queue
- **RAII**: Resource management with smart pointers

## 📁 Project Structure

```
MyTInyWebServer/
├── include/              # Core header files
│   ├── webserver.hpp    # Main server class
│   ├── epoller.hpp      # Event listener
│   ├── threadpool.hpp   # Thread pool implementation
│   └── include.hpp      # Common includes
├── src/                 # Source code implementation
│   ├── main.cpp         # Entry point
│   ├── webserver.cpp    # Server implementation
│   ├── epoller.cpp      # Event handling
│   ├── threadpool.cpp   # Thread pool logic
│   ├── httpconnector.cpp # HTTP connection handling
│   ├── buffer.cpp       # Buffer management
│   ├── httprequest.cpp  # HTTP request parsing
│   ├── httpresponse.cpp # HTTP response generation
│   ├── httpdata.cpp     # HTTP data structures
│   ├── log.cpp          # Logging system
│   └── sql_pool.cpp     # Database connection pool
├── httpconnector/       # HTTP connection components
├── buffer/              # Buffer management
├── log/                 # Logging system
├── sqlpool/             # Database connection pool
├── outheader/           # MySQL headers
├── CMakeLists.txt       # Build configuration
└── README.md            # Project documentation
```

## 🛠️ Prerequisites

- **C++17 compatible compiler** (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake 3.10+**
- **MySQL development libraries**
- **Linux operating system** (for epoll support)

## 🔧 Building

### 1. Clone the repository
```bash
git clone <repository-url>
cd MyTInyWebServer
```

### 2. Create build directory
```bash
mkdir build && cd build
```

### 3. Configure and build
```bash
cmake ..
make
```

### 4. Run the server
```bash
./MyTinyWebServer
```

## 🚀 Usage

### Basic Server Setup

```cpp
#include "../include/webserver.hpp"

int main(int argc, char **argv)
{
    // Create WebServer instance
    // Parameters: port, max_db_connections, host, user, password, database, db_port
    WebServer* ws = WebServer::GetInstance(9096, 10, "127.0.0.1", 
                                          "root", "password", "yourdb", 3306);
    
    // Start the server
    ws->Start();
    
    return 0;
}
```

### Configuration Options

- **Port**: Server listening port (default: 9096)
- **Database Connections**: Maximum concurrent database connections
- **Database Host**: MySQL server address
- **Thread Pool Size**: Configurable worker thread count
- **Event Wait Timeout**: Epoll wait timeout configuration

## 📊 Performance Characteristics

- **Concurrent Connections**: Supports thousands of concurrent connections
- **Memory Efficiency**: Optimized buffer management and resource usage
- **CPU Utilization**: Efficient event-driven processing with minimal CPU overhead
- **Scalability**: Horizontal scaling capability through load balancing

## 🔍 Key Features Explained

### Event-Driven Architecture
The server uses Linux epoll for efficient event handling, allowing it to manage thousands of concurrent connections with minimal resource overhead.

### Thread Pool Design
A configurable thread pool handles HTTP request processing, ensuring optimal resource utilization and preventing thread explosion.

### Buffer Management
Dual-buffer system (read/write) with efficient memory management for optimal I/O performance.

### Database Integration
Built-in MySQL connection pool for efficient database operations and connection reuse.

## 🧪 Testing

The server can be tested using:
- **Web browsers** for HTTP requests
- **Apache Bench (ab)** for load testing
- **curl** for API testing
- **Custom HTTP clients** for specific testing scenarios

## 📈 Performance Testing

```bash
# Example load testing with Apache Bench
ab -n 10000 -c 100 http://localhost:9096/
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Linux epoll API for efficient event handling
- MySQL C API for database connectivity
- Modern C++ community for best practices
- Open source community for inspiration and tools

## 🔮 Future Enhancements

- [ ] HTTPS/SSL support
- [ ] WebSocket support
- [ ] Configuration file support
- [ ] Metrics and monitoring
- [ ] Docker containerization
- [ ] Windows compatibility
- [ ] Load balancing support
- [ ] Caching mechanisms

---

**Built with ❤️ using modern C++ and high-performance networking principles**
