#include "../include/webserver.hpp"

using namespace std;
int main(int argc, char ** argv)
{
    WebServer* ws=WebServer::GetInstance(9096,10,"127.0.0.1","root","1357488","yourdb",3306); //创建一个webserver实例，监听9090端口
    ws->Start(); //启动webserver
}