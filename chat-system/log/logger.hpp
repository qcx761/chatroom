#ifndef SIMPLELOGGER_H
#define SIMPLELOGGER_H

#include <string>

class SimpleLogger {
public:
    enum class Level { ERROR, WARN, INFO, DEBUG };

    SimpleLogger(Level level = Level::INFO, const std::string& filename = "");
    ~SimpleLogger();

    void log(Level level, const std::string& msg);
    void setLevel(Level level);

private:
    class Impl;
    Impl* pImpl;  // Pimpl 习惯写法，防止实现暴露（可选）
};

// 方便调用的宏
#define LOG_ERROR(logger, msg) (logger).log(SimpleLogger::Level::ERROR, (msg))
#define LOG_WARN(logger, msg)  (logger).log(SimpleLogger::Level::WARN,  (msg))
#define LOG_INFO(logger, msg)  (logger).log(SimpleLogger::Level::INFO,  (msg))
#define LOG_DEBUG(logger, msg) (logger).log(SimpleLogger::Level::DEBUG, (msg))

#endif // SIMPLELOGGER_H
