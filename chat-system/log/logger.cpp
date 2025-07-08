#include "logger.hpp"
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>

class SimpleLogger::Impl {
public:
    Impl(Level level, const std::string& filename)
        : logLevel(level)
    {
        if (!filename.empty()) {
            fileStream.open(filename, std::ios::app);
            if (!fileStream.is_open()) {
                std::cerr << "无法打开日志文件：" << filename << std::endl;
            }
        }
    }

    ~Impl() {
        if (fileStream.is_open()) {
            fileStream.close();
        }
    }

    void log(Level level, const std::string& msg) {
        if (level > logLevel) return;

        std::lock_guard<std::mutex> lock(mtx);

        std::string logLine = "[" + currentTime() + "] " + levelToString(level) + ": " + msg;

        std::cout << logLine << std::endl;

        if (fileStream.is_open()) {
            fileStream << logLine << std::endl;
        }
    }

    void setLevel(Level level) {
        logLevel = level;
    }

private:
    Level logLevel;
    std::mutex mtx;
    std::ofstream fileStream;

    std::string currentTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        char buf[20];
#ifdef _WIN32
        ctime_s(buf, sizeof(buf), &t);
#else
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
#endif
        return std::string(buf);
    }

    std::string levelToString(Level level) {
        switch(level) {
            case Level::ERROR: return "ERROR";
            case Level::WARN:  return "WARN ";
            case Level::INFO:  return "INFO ";
            case Level::DEBUG: return "DEBUG";
            default: return "UNK";
        }
    }
};

// SimpleLogger 构造、析构、调用转发
SimpleLogger::SimpleLogger(Level level, const std::string& filename)
    : pImpl(new Impl(level, filename))
{}

SimpleLogger::~SimpleLogger() {
    delete pImpl;
}

void SimpleLogger::log(Level level, const std::string& msg) {
    pImpl->log(level, msg);
}

void SimpleLogger::setLevel(Level level) {
    pImpl->setLevel(level);
}
