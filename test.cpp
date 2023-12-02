#include "poll/poll.hpp"
#include <thread>
#include <algorithm>

using namespace std;
using namespace std::chrono;

void noPoll(int range)
{
    for (int i = 0; i < range; i++)
    {
        mysql::Conn conn("hwy", "xj", "Qwert123.", "user", 3306);
        char buff[100]{0};
        snprintf(buff, sizeof buff, "insert test(name,pwd) values('zs','123')");
        int line;
        conn.insert(buff, &line);
    }
}

void usePoll(mysql::Poll &poll, int range)
{
    for (int i = 0; i < range; i++)
    {
        auto conn = poll.getConn();
        char buff[128]{0};
        snprintf(buff, sizeof buff, "insert test(name,pwd) values('zs','123')");
        int line;
        conn->insert(buff, &line);
    }
}

// 单线程
void oneThread()
{
    steady_clock::time_point begin, end;
    begin = steady_clock::now();
#if 1 // 不适用连接池
    noPoll(1000);
#endif

#if 0 // 使用连接池
    mysql::Poll::libraryInit("/home/xj/projects/mysqlConnPoll_CPP/poll/config.json");
    mysql::Poll &poll = mysql::Poll::getInstance();
    usePoll(poll,1000);
#endif
    end = steady_clock::now();
    std::cout << duration_cast<milliseconds>(end - begin).count() << std::endl;
}

// 多线程
void mutThread()
{
    steady_clock::time_point begin, end;
    begin = steady_clock::now();
#if 0 // 不适用连接池
    std::thread t1(noPoll,250);
    std::thread t2(noPoll,250);
    std::thread t3(noPoll,250);
    std::thread t4(noPoll,250);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
#endif

#if 1 // 使用连接池
    mysql::Poll::libraryInit("/home/xj/projects/mysqlConnPoll_CPP/poll/config.json");
    mysql::Poll &poll = mysql::Poll::getInstance();
    std::thread t1(usePoll,std::ref(poll),250);
    std::thread t2(usePoll,std::ref(poll),250);
    std::thread t3(usePoll,std::ref(poll),250);
    std::thread t4(usePoll,std::ref(poll),250);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
#endif
    end = steady_clock::now();
    std::cout << duration_cast<milliseconds>(end - begin).count() << "ms" << std::endl;
}

int main()
{
    oneThread();
    mutThread();
    return 0;
}
