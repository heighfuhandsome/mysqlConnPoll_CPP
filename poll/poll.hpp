#pragma once
#include "conn.hpp"
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>
#include <json/json.h>
#include <chrono>
#include <atomic>
#include <list>

// 设计为单例模式
namespace mysql
{
    class Poll
    {
    public:
        // 这个参数从 main函数的argv中传入 
        static void libraryInit(std::string configPath = "./mysqlConnConf.txt");
        static Poll &getInstance();

        Poll(const Poll &) = delete;
        Poll &operator=(const Poll &) = delete;
        ~Poll();
        std::shared_ptr<Conn> getConn();

        uint32_t getCoonNum() {  return allConnSize_;}
    private:
        Poll();
        static Json::Value config_;
        std::shared_ptr<Conn> makeShared(Conn *conn);
        /*
        {
            "host"    : "主机/ip",
            "user"    : "mysql账户",
            "pwd"     : "mysql密码",
            "db"      : "要连接的数据库名称",
            "port"    : mysql端口号,
            "maxSize" : 最大连接个数,
            "minSize" : 最小连接个数,
            "timeout" : 连接最大空闲市场
        }
        */
        std::mutex mutex_;
        std::condition_variable cond_;
        std::list<mysql::Conn *> poll_;
        //总连接数
        std::atomic_uint32_t allConnSize_;
        Conn *addConn();
        void recycleConn();
    };

}