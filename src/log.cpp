#include "log.hpp"

int Log::_statue; //日志状态
mutex Log::_bqMutex;//缓冲区锁
unique_ptr<BlockDeque<string>> Log::_bq;//缓冲区阻塞队列
ofstream Log::_logFile;//日志文件 

static std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);

    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d");
    return ss.str();
}

Log::Log()
{
    _bq=nullptr; //初始化缓冲区
    _statue=log_close; //初始化状态
    //_logFile.open(_LogFileName, std::ios::app); // 打开文件以追加模式
}

Log::~Log()
{
    _statue=log_close; //关闭日志
    _bq->Close(); //关闭缓冲区
}

void Log::Init(const size_t& cap)
{
    _statue=log_open;//日志状态

    unique_ptr<BlockDeque<string>> newDeque(new BlockDeque<std::string>(cap));
    _bq = move(newDeque);//初始化缓冲区阻塞队列

    _LogFileName=log_path_head+getTimestamp()+"_server_log.txt"; //初始化文件名
    _logFile.open(_LogFileName, std::ios::app);

    unique_ptr<thread> NewThread(new thread(LogThread)); //创建日志线程
    _threadPtr = move(NewThread);

    _threadPtr->detach(); //分离线程

    //输入换页
    std::unique_lock<std::mutex> locker(_bqMutex);
    _logFile << std::endl;
    _logFile << "-------------------------------------------------------------------------------------" << std::endl; //输出到文件

}

void Log::LogThread()
{
    while (true)
    {
        if(_statue==log_close)
            return;
        string str="";
        if(_bq->pop_front(str)) //从阻塞队列中取出一个日志
        {
            std::cout << str << std::endl; //输出到控制台
            std::unique_lock<std::mutex> locker(_bqMutex);
            //_logFile << str << std::endl; //输出到文件
        }

        // std::unique_lock<std::mutex> locker(_buffMutex);
        // while(_buff.empty())
        // {
        //     std::cout << "wait..."<<std::endl;
        //     if(_statue==log_close)
        //         return;
        //     _CVbuff.wait(locker); //等待缓冲区有数据
        // }
        // std::string str = _buff.front();
        // _buff.pop(); //从队列中取出一个日志
        
    }
}

void Log::Write(const string& msg)
{
    
    _bq->push_back(msg); //将日志放入缓冲区
    // std::unique_lock<std::mutex> locker(_buffMutex); //加锁
    // _buff.push(msg);
    // _CVbuff.notify_one(); //通知日志线程
}

void Log::Flush()
{
    std::unique_lock<std::mutex> locker(_bqMutex);
    _logFile.flush();
    std::cout.flush();
}

string LogStr(const string& level,const string& message,const string& filename,long line)
{
    //日志格式:[日志级别][时间戳][线程号](文件名:行号) 日志内容
    time_t t;
    time(&t);
    std::string timeStr = ctime(&t);   
    // 去掉 ctime 返回值中的换行符
    if (!timeStr.empty() && timeStr.back() == '\n') {
        timeStr.pop_back();
    }
    std::thread::id this_id = std::this_thread::get_id();
    std::stringstream logStream;
    logStream << "[" << level << "]["<<timeStr<< "]["<<"thread:"<<this_id<<"](" << filename << ":" << line << ") "
              << message;

    return logStream.str();
}


