#pragma once
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <chrono>

namespace mysql
{
    class Conn
    {
    public:
        Conn(const char *host, const char *user, const char *pwd, const char *dbName, unsigned int port);
        ~Conn();
        Conn(const Conn &) = delete;
        Conn &operator=(const Conn &) = delete;
        bool isConnected() const { return connected; };
        // line 传出参数，表示 insert/del/update 后所影响的行数
        bool insert(const std::string &sql, int *line) const { return exec(sql, line); };
        bool remove(const std::string &sql, int *line) const { return exec(sql, line); };
        bool update(const std::string &sql, int *line) const { return exec(sql, line); };
        // line 传出参数，表示查询到的行数
        bool query(const std::string &sql, int *line);
        // 遍历获取查询后的行数
        const std::vector<const char *> next();
        // 获取表字段
        const std::vector<const char *> getIndex();
        void freeResult()
        {
            if (result)
            {
                mysql_free_result(result);
                result = nullptr;
            }
        };
        // 开启事务
        void onTransaction() { mysql_autocommit(mysql, false); };
        // 关闭事务
        void offTransaction() { mysql_autocommit(mysql, true); };
        // 提交事务
        void commit() const { mysql_commit(mysql); }
        // 回滚事务
        void rollback() const { mysql_rollback(mysql); }
        const char *getError() const { return error; }
        // 刷新连接最后一次放入连接池的时间
        void reflushAliveTime() { aliveTime_ = std::chrono::steady_clock::now(); }
        // 获取连接空闲时长 返回毫秒
        unsigned int getLeisureTime();

    private:
        bool exec(const std::string &sql, int *line) const;

        MYSQL *mysql;
        MYSQL_RES *result;
        const char *error;
        bool connected;
        // 最后一次被放回连接池的时间点
        std::chrono::steady_clock::time_point aliveTime_;
    };
};