# MyTInyWebServer

一个基于现代C++17开发的高性能并发HTTP Web服务器，具有出色的并发处理能力和高效的资源管理。

## 🚀 项目特性

- **高并发性能**: 采用Epoll + 线程池架构，提供卓越的并发处理能力
- **现代C++**: 使用C++17标准，采用现代编程实践
- **事件驱动**: 非阻塞I/O配合高效的事件驱动编程
- **数据库集成**: 内置MySQL连接池支持
- **完整日志系统**: 功能齐全的日志系统，支持调试和监控
- **缓冲区管理**: 高效的I/O缓冲区处理，采用双缓冲区设计
- **HTTP协议**: 完整的HTTP请求/响应处理
- **跨平台潜力**: 基于Linux，具备跨平台扩展潜力

## 🏗️ 架构设计

### 核心组件

- **WebServer**: 主服务器类，实现单例模式
- **Epoller**: 基于Linux epoll的高性能事件监听器
- **ThreadPool**: 可配置的工作线程池，用于任务执行
- **HttpConnector**: HTTP连接处理器，处理请求/响应
- **Buffer**: 高效的I/O缓冲区管理系统
- **SQLPool**: MySQL数据库连接池
- **Logger**: 完整的日志系统

### 设计模式

- **单例模式**: 确保WebServer实例的唯一性
- **事件驱动架构**: 使用epoll进行高效的事件处理
- **生产者-消费者**: 线程池配合任务队列
- **RAII**: 使用智能指针进行资源管理

## 📁 项目结构

```
MyTInyWebServer/
├── include/              # 核心头文件
│   ├── webserver.hpp    # 主服务器类
│   ├── epoller.hpp      # 事件监听器
│   ├── threadpool.hpp   # 线程池实现
│   └── include.hpp      # 通用包含文件
├── src/                 # 源代码实现
│   ├── main.cpp         # 程序入口
│   ├── webserver.cpp    # 服务器实现
│   ├── epoller.cpp      # 事件处理
│   ├── threadpool.cpp   # 线程池逻辑
│   ├── httpconnector.cpp # HTTP连接处理
│   ├── buffer.cpp       # 缓冲区管理
│   ├── httprequest.cpp  # HTTP请求解析
│   ├── httpresponse.cpp # HTTP响应生成
│   ├── httpdata.cpp     # HTTP数据结构
│   ├── log.cpp          # 日志系统
│   └── sql_pool.cpp     # 数据库连接池
├── httpconnector/       # HTTP连接组件
├── buffer/              # 缓冲区管理
├── log/                 # 日志系统
├── sqlpool/             # 数据库连接池
├── outheader/           # MySQL头文件
├── CMakeLists.txt       # 构建配置
└── README.md            # 项目文档
```

## 🛠️ 系统要求

- **C++17兼容编译器** (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake 3.10+**
- **MySQL开发库**
- **Linux操作系统** (支持epoll)

## 🔧 构建步骤

### 1. 克隆仓库
```bash
git clone <仓库地址>
cd MyTInyWebServer
```

### 2. 创建构建目录
```bash
mkdir build && cd build
```

### 3. 配置和构建
```bash
cmake ..
make
```

### 4. 运行服务器
```bash
./MyTinyWebServer
```

## 🚀 使用方法

### 基本服务器设置

```cpp
#include "../include/webserver.hpp"

int main(int argc, char **argv)
{
    // 创建WebServer实例
    // 参数: 端口, 最大数据库连接数, 主机, 用户, 密码, 数据库, 数据库端口
    WebServer* ws = WebServer::GetInstance(9096, 10, "127.0.0.1", 
                                          "root", "密码", "数据库名", 3306);
    
    // 启动服务器
    ws->Start();
    
    return 0;
}
```

### 配置选项

- **端口**: 服务器监听端口 (默认: 9096)
- **数据库连接数**: 最大并发数据库连接数
- **数据库主机**: MySQL服务器地址
- **线程池大小**: 可配置的工作线程数量
- **事件等待超时**: Epoll等待超时配置

## 📊 性能特征

- **并发连接**: 支持数千个并发连接
- **内存效率**: 优化的缓冲区管理和资源使用
- **CPU利用率**: 高效的事件驱动处理，CPU开销最小
- **可扩展性**: 通过负载均衡实现水平扩展

## 🔍 核心功能详解

### 事件驱动架构
服务器使用Linux epoll进行高效的事件处理，能够以最小的资源开销管理数千个并发连接。

### 线程池设计
可配置的线程池处理HTTP请求，确保资源的最优利用，防止线程爆炸。

### 缓冲区管理
双缓冲区系统（读/写），高效的内存管理，实现最优的I/O性能。

### 数据库集成
内置MySQL连接池，实现高效的数据库操作和连接复用。

## 🧪 测试方法

服务器可以通过以下方式进行测试：
- **Web浏览器** 进行HTTP请求测试
- **Apache Bench (ab)** 进行负载测试
- **curl** 进行API测试
- **自定义HTTP客户端** 进行特定场景测试

## 📈 性能测试

```bash
# 使用Apache Bench进行负载测试示例
ab -n 10000 -c 100 http://localhost:9096/
```

## 🤝 贡献指南

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/新功能`)
3. 提交更改 (`git commit -m '添加新功能'`)
4. 推送到分支 (`git push origin feature/新功能`)
5. 创建 Pull Request

## 📝 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙏 致谢

- Linux epoll API 提供高效的事件处理
- MySQL C API 提供数据库连接功能
- 现代C++社区提供最佳实践
- 开源社区提供灵感和工具

## 📞 技术支持

如有问题、建议或贡献，请：
- 在GitHub上创建issue
- 查看现有文档
- 查看源代码了解实现细节

## 🔮 未来功能规划

- [ ] HTTPS/SSL支持
- [ ] WebSocket支持
- [ ] 配置文件支持
- [ ] 指标和监控
- [ ] Docker容器化
- [ ] Windows兼容性
- [ ] 负载均衡支持
- [ ] 缓存机制

## 💡 技术亮点

### 高并发设计
- **Epoll机制**: 使用Linux内核的epoll接口，支持大量文件描述符的高效监控
- **非阻塞I/O**: 避免线程阻塞，提高并发处理能力
- **事件驱动**: 基于事件的编程模型，响应迅速

### 内存管理
- **智能指针**: 使用RAII原则，自动管理资源生命周期
- **缓冲区优化**: 双缓冲区设计，减少内存拷贝
- **连接池**: 复用数据库连接，降低资源开销

### 性能优化
- **线程池**: 避免频繁创建销毁线程
- **零拷贝**: 优化数据传输路径
- **异步处理**: 提高I/O效率

## 🎯 适用场景

- **高并发Web服务**: 适合需要处理大量并发请求的场景
- **静态文件服务器**: 高效的静态资源分发
- **轻量级API服务**: 快速响应的API接口
- **学习研究**: 高并发编程和网络编程的学习项目
- **中小型应用**: 适合中小规模的Web应用部署

---

**使用现代C++和高性能网络编程原理构建，用心打造 ❤️**
