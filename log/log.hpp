#pragma once
#include "blockqueue.hpp"
#include "../include/include.hpp"

using std::string;
using std::thread;
using std::mutex;
using std::unique_ptr;
using std::unique_lock;
using std::condition_variable;
using std::ofstream;

const string log_path_head="../out/log/";//Log directory.



#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define RELATIVE_PATH(file) (std::string(file).substr(std::string(SOURCE_ROOT).length() + 1))//Strip the source root prefix.


enum LogLevel
{
	INFO,
	WARNING,
	FATAL,
    ERROR
};

enum Log_statue
{
    log_open,
    log_close,
};

class Log
{
    public:
    //Function-local singleton initialization is thread safe.
    static Log* GetInstance()
    {
        static Log log;
        return &log;
    }

    private:
    // Private constructor.
    Log();
    ~Log();
    // Disable copying and assignment.
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    public:
    void Write(const string& msg);
    void Init(const size_t &cap); // Initialize request state.
    void Flush();
    static void LogThread();// Logging worker.
    bool OpenCheck()
    {
        return _statue == log_open;
    };

private:
    static int _statue;                // Logging state.
    unique_ptr<thread> _threadPtr;     // Logging thread.
    static unique_ptr<BlockDeque<string>> _bq;//Blocking log queue.
    static mutex _bqMutex;//Log queue mutex.
    static ofstream _logFile;//Log stream.
    string _LogFileName;//Log filename.
};

string LogStr(const string& level,const string& message,const string& filename,long line);

#define LOG(message, level) LogStr(#level,message,RELATIVE_PATH(__FILE__),__LINE__)

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::GetInstance();\
        if (log->OpenCheck()) {\
            char buffer[1024]; \
            snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
            log->Write(LogStr(#level, std::string(buffer),RELATIVE_PATH(__FILE__),__LINE__)); \
            log->Flush();\
        }\
    } while(0);

#define LOG_INFO(format, ...) do {LOG_BASE(INFO, format, ##__VA_ARGS__)} while(0);
#define LOG_FATAL(format, ...) do {LOG_BASE(FATAL, format, ##__VA_ARGS__)} while(0);
#define LOG_WARNING(format, ...) do {LOG_BASE(WARNING, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(ERROR, format, ##__VA_ARGS__)} while(0);
