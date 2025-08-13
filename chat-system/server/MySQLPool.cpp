#include "MySQLPool.hpp"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <iostream>

MySQLPool::MySQLPool(const std::string& url,
                     const std::string& user,
                     const std::string& password,
                     const std::string& schema,
                     int pool_size)
    : url_(url), user_(user), password_(password), schema_(schema), pool_size_(pool_size)
{
    driver_ = sql::mysql::get_mysql_driver_instance();
    if (!driver_) throw std::runtime_error("Failed to get MySQL driver");

    for (int i = 0; i < pool_size_; ++i) {
        sql::Connection* conn = driver_->connect(url_, user_, password_);
        conn->setSchema(schema_);
        connections_.push(conn);
    }
}

MySQLPool::~MySQLPool() {
    std::lock_guard<std::mutex> lock(mtx_);
    while (!connections_.empty()) {
        sql::Connection* conn = connections_.front();
        connections_.pop();
        conn->close();
        delete conn;
    }
}

std::shared_ptr<sql::Connection> MySQLPool::getConnection() {
    std::unique_lock<std::mutex> lock(mtx_);
    cond_.wait(lock, [this]() { return !connections_.empty(); });
    sql::Connection* conn = connections_.front();
    connections_.pop();

    return std::shared_ptr<sql::Connection>(conn, [this](sql::Connection* c) {
        std::lock_guard<std::mutex> lock(this->mtx_);
        this->connections_.push(c);
        this->cond_.notify_one();
    });
}
