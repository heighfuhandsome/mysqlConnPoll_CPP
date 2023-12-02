#include "conn.hpp"

using namespace mysql;
Conn::Conn(const char *host, const char *user, const char *pwd, const char *dbName, unsigned int port)
    : connected(false), result(nullptr), mysql(nullptr)
{
    mysql = mysql_init(nullptr);
    if (mysql_real_connect(mysql, host, user, pwd, dbName, port, nullptr, 0))
    {
        connected = true;
    }
    else
    {
        error = mysql_error(mysql);
    }
}

Conn::~Conn()
{
    if (mysql)
    {
        if (result)
        {
            mysql_free_result(result);
            result = nullptr;
        }
        mysql_close(mysql);
    }
    mysql = nullptr;
}

bool Conn::query(const std::string &sql, int *line)
{
    if (result)
    {
        mysql_free_result(result);
        result = nullptr;
    }
    if (!mysql_real_query(mysql, sql.c_str(), sql.size()))
    {
        result = mysql_store_result(mysql);
        *line = mysql_num_rows(result);
        return true;
    }
    return false;
}

const std::vector<const char *> Conn::next()
{
    std::vector<const char *> data;
    const MYSQL_ROW &row = mysql_fetch_row(result);
    int num = mysql_num_fields(result);
    for (int i = 0; i < num; i++)
    {
        data.push_back(row[i]);
    }
    return data;
}

const std::vector<const char *> Conn::getIndex()
{
    std::vector<const char *> fields;
    if (result)
    {
        MYSQL_FIELD *fs = mysql_fetch_fields(result);
        int num = mysql_num_fields(result);
        for (int i = 0; i < num; i++)
        {
            fields.push_back(fs[i].name);
        }
    }
    return fields;
}

unsigned int mysql::Conn::getLeisureTime()
{
    using namespace std::chrono;
    auto now = steady_clock::now();
    return duration_cast<milliseconds>(now - aliveTime_).count();
}

bool Conn::exec(const std::string &sql, int *line) const
{
    if (!mysql_real_query(mysql, sql.c_str(), sql.size()))
    {
        *line = mysql_affected_rows(mysql);
        return true;
    }
}
