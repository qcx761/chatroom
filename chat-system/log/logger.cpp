#include "logger.hpp"
#include <fstream>
#include <iostream>
#include <ctime>
#include <mutex>

// 日志实现类
class Logger::Impl {
public:
    Impl(Level level, const std::string& filename)
        : m_level(level), m_filename(filename) 
    {
        if (!m_filename.empty()) {
            m_file.open(m_filename, std::ios::app);
            if (!m_file.is_open()) {
                std::cerr << "无法打开日志文件：" << m_filename << std::endl;
            }
        }
    }

    ~Impl() {
        if (m_file.is_open()) {
            m_file.close();
        }
    }

    void log(Level level, const std::string& msg) {
        if (level > m_level) return;

        std::lock_guard<std::mutex> lock(m_mutex);

        std::string levelStr = levelToString(level);
        std::string timeStr = currentDateTime();

        std::string logMsg = "[" + timeStr + "] [" + levelStr + "] " + msg + "\n";

        if (m_file.is_open()) {
            m_file << logMsg;
            m_file.flush();
        } else {
            std::cerr << logMsg;
        }
    }

    void setLevel(Level level) {
        m_level = level;
    }

private:
    Level m_level;
    std::string m_filename;
    std::ofstream m_file;
    std::mutex m_mutex;

    std::string currentDateTime() {
        std::time_t now = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return buf;
    }

    std::string levelToString(Level level) {
        switch(level) {
            case Level::ERROR: return "ERROR";
            case Level::WARN:  return "WARN";
            case Level::INFO:  return "INFO";
            case Level::DEBUG: return "DEBUG";
            default: return "UNKNOWN";
        }
    }
};

// Logger 成员函数实现

Logger::Logger(Level level, const std::string& filename)
    : pImpl(new Impl(level, filename)) {}

Logger::~Logger() {
    delete pImpl;
}

void Logger::log(Level level, const std::string& msg) {
    pImpl->log(level, msg);
}

void Logger::setLevel(Level level) {
    pImpl->setLevel(level);
}

// 定义全局 logger 对象，默认 DEBUG 级别，写到 chat.log
Logger logger(Logger::Level::DEBUG, "chat.log");
