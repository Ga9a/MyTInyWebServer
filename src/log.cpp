#include "log.hpp"

int Log::_statue; //Logging state.
mutex Log::_bqMutex;//Buffer mutex.
unique_ptr<BlockDeque<string>> Log::_bq;//Blocking log buffer.
ofstream Log::_logFile;//Log stream.

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
    _bq=nullptr; //Initialize the log queue.
    _statue=log_close; //Initialize logging state.
    //_logFile.open(_LogFileName, std::ios::app); // Open the log file for append.
}

Log::~Log()
{
    _statue=log_close; //Disable logging.
    _bq->Close(); //Close the queue.
}

void Log::Init(const size_t& cap)
{
    _statue=log_open;//Logging state.

    unique_ptr<BlockDeque<string>> newDeque(new BlockDeque<std::string>(cap));
    _bq = std::move(newDeque);//Initialize the blocking queue.

    _LogFileName=log_path_head+getTimestamp()+"_server_log.txt"; //Set the log filename.
    _logFile.open(_LogFileName, std::ios::app);

    unique_ptr<thread> NewThread(new thread(LogThread)); //Start the logging worker.
    _threadPtr = std::move(NewThread);

    _threadPtr->detach(); //Detach the worker.

    //Write a log separator.
    std::unique_lock<std::mutex> locker(_bqMutex);
    _logFile << std::endl;
    _logFile << "-------------------------------------------------------------------------------------" << std::endl; //Write to the log file.

}

void Log::LogThread()
{
    while (true)
    {
        if(_statue==log_close)
            return;
        string str="";
        if(_bq->pop_front(str)) //Read the next queued message.
        {
            std::cout << str << std::endl; //Write to standard output.
            std::unique_lock<std::mutex> locker(_bqMutex);
            //_logFile << str << std::endl; //Write to the log file.
        }

        // std::unique_lock<std::mutex> locker(_buffMutex);
        // while(_buff.empty())
        // {
        //     std::cout << "wait..."<<std::endl;
        //     if(_statue==log_close)
        //         return;
        //     _CVbuff.wait(locker); //Wait for queued data.
        // }
        // std::string str = _buff.front();
        // _buff.pop(); //Pop the next log message.

    }
}

void Log::Write(const string& msg)
{

    _bq->push_back(msg); //Enqueue the log message.
    // std::unique_lock<std::mutex> locker(_buffMutex); //Acquire the mutex.
    // _buff.push(msg);
    // _CVbuff.notify_one(); //Notify the logging worker.
}

void Log::Flush()
{
    std::unique_lock<std::mutex> locker(_bqMutex);
    _logFile.flush();
    std::cout.flush();
}

string LogStr(const string& level,const string& message,const string& filename,long line)
{
    //Log format: [level][time][thread](file:line) message.
    time_t t;
    time(&t);
    std::string timeStr = ctime(&t);
    // Remove the trailing newline from ctime.
    if (!timeStr.empty() && timeStr.back() == '\n') {
        timeStr.pop_back();
    }
    std::thread::id this_id = std::this_thread::get_id();
    std::stringstream logStream;
    logStream << "[" << level << "]["<<timeStr<< "]["<<"thread:"<<this_id<<"](" << filename << ":" << line << ") "
              << message;

    return logStream.str();
}


