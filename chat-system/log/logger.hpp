#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

class Logger {
public:
    enum class Level { ERROR, WARN, INFO, DEBUG };

    Logger(Level level = Level::INFO, const std::string& filename = "");
    ~Logger();

    void log(Level level, const std::string& msg);
    void setLevel(Level level);

private:
    class Impl;
    Impl* pImpl;
};

// 日志宏方便调用
#define LOG_ERROR(logger, msg) (logger).log(Logger::Level::ERROR, (msg))
#define LOG_WARN(logger, msg)  (logger).log(Logger::Level::WARN,  (msg))
#define LOG_INFO(logger, msg)  (logger).log(Logger::Level::INFO,  (msg))
#define LOG_DEBUG(logger, msg) (logger).log(Logger::Level::DEBUG, (msg))

// 全局 logger 对象声明
extern Logger logger;

#endif // LOGGER_HPP
