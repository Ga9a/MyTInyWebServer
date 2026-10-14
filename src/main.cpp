#include "../include/webserver.hpp"
#include <cstdlib>
#include <csignal>
namespace {
const char* environment(const char* name, const char* fallback) {
    const char* value = std::getenv(name);
    return value && *value ? value : fallback;
}
int number(const char* value, int minimum, int maximum) {
    std::size_t end = 0;
    int result = std::stoi(value, &end);
    if (value[end] != '\0' || result < minimum || result > maximum)
        throw std::invalid_argument("Configuration number is outside the allowed range");
    return result;
}
}
int main(int argc, char** argv) {
    try {
        if (argc > 2) {
            std::cerr << "Usage: " << argv[0] << " [port]\n";
            return 2;
        }
        const char* password = std::getenv("MYSQL_PASSWORD");
        if (!password) throw std::invalid_argument("Set MYSQL_PASSWORD before starting the server");
        const int port = number(argc == 2 ? argv[1] : environment("PORT", "9096"), 1024, 65535);
        std::signal(SIGPIPE, SIG_IGN);
        WebServer::GetInstance(port, number(environment("MYSQL_POOL_SIZE", "10"), 1, 1000),
            environment("MYSQL_HOST", "127.0.0.1"), environment("MYSQL_USER", "webserver"),
            password, environment("MYSQL_DATABASE", "webserver"),
            number(environment("MYSQL_PORT", "3306"), 1, 65535))->Start();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Startup failed: " << error.what() << '\n';
        return 1;
    }
}
