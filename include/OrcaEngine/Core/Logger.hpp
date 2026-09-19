#pragma once

#include <string>
#include <vector>

enum class LogLevel  {
    Info,
    Warning,
    Error
};

struct LogMessage {
    LogLevel level;
    std::string message;
};

#include <vector>

class Logger {
public:
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);

    static std::vector<LogMessage>& GetMessages() { return _messages; }
    static void Clear();
private:
    static std::vector<LogMessage> _messages; 
}; 