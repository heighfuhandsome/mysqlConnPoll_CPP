#include "poll.hpp"
#include <fstream>
#include <thread>
#include <functional>

Json::Value mysql::Poll::config_;

void mysql::Poll::libraryInit(std::string configPath)
{
    Json::Reader reader;
    std::ifstream in(configPath);
    reader.parse(in, config_, false);
}

mysql::Poll &mysql::Poll::getInstance()
{
    static Poll poll;
    return poll;
}

mysql::Poll::~Poll()
{
    while (!poll_.empty())
    {
        delete poll_.front();
        poll_.pop_front();
    }
}

std::shared_ptr<mysql::Conn> mysql::Poll::getConn()
{
    std::unique_lock<std::mutex> ulock(mutex_);

    while (poll_.empty())
    {
        std::cv_status stat = cond_.wait_for(ulock, std::chrono::microseconds(100));
        if(stat == std::cv_status::timeout && allConnSize_ < config_["maxSize"].asUInt() )
        {
            auto conn = addConn();
            if(conn) return makeShared(conn); else continue;
        }
    }
    auto conn = poll_.front();
    poll_.pop_front();
    return makeShared(conn);
}

mysql::Poll::Poll():allConnSize_(0)
{
    for (size_t i = 0; i < config_["minSize"].asUInt(); i++)
    {
        auto ptr = addConn();
        if (ptr)
        {
            poll_.push_back(ptr);
        }
    }

    // 用于回收长时间空闲的连接
    std::thread t(std::bind(&mysql::Poll::recycleConn, this));
    t.detach();
}

std::shared_ptr<mysql::Conn> mysql::Poll::makeShared(Conn *conn)
{
    std::shared_ptr<Conn> ptr(conn, [&](Conn *conn)
                              {
        conn->reflushAliveTime();
        std::lock_guard<std::mutex> guard(mutex_);
        poll_.push_back(conn); 
        cond_.notify_one(); });
    return ptr;
}

mysql::Conn *mysql::Poll::addConn()
{
    auto host = config_["host"].asString();
    auto user = config_["user"].asString();
    auto pwd = config_["pwd"].asString();
    auto db = config_["db"].asString();
    auto port = config_["port"].asUInt();
    Conn *conn = new Conn(host.c_str(), user.c_str(), pwd.c_str(), db.c_str(), port);
    if (!conn->isConnected())
    {
        fprintf(stderr, "mysql conn fail: %s", conn->getError());
        delete conn;
        return nullptr;
    }
    conn->reflushAliveTime();
    allConnSize_++;
    return conn;
}

void mysql::Poll::recycleConn()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto b = poll_.begin(); b != poll_.end();)
        {
            if (allConnSize_ > config_["minSize"].asUInt() && (*b)->getLeisureTime() >= config_["timeout"].asUInt())
            {
                allConnSize_-- ;
                delete *b;
                b = poll_.erase(b);
            }
            else
            {
                b++;
            }
        }
    }
}
