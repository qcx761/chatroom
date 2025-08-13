#pragma once
#include <mysql_driver.h> // 提供 sql::mysql::MySQL_Driver
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <stdexcept>

class MySQLPool {
public:
    MySQLPool(const std::string& url,
              const std::string& user,
              const std::string& password,
              const std::string& schema,
              int pool_size);

    ~MySQLPool();

    std::shared_ptr<sql::Connection> getConnection();

private:
    std::string url_, user_, password_, schema_;
    int pool_size_;
    sql::mysql::MySQL_Driver* driver_;

    std::queue<sql::Connection*> connections_;
    std::mutex mtx_;
    std::condition_variable cond_;
};
