#include <OrcaEngine/Core/Logger.hpp>

#include <iostream>

std::vector<LogMessage> Logger::_messages;

void Logger::Info(const std::string& message)
{
    _messages.push_back({ LogLevel::Info, message });
    std::cout << "[Info] " << message << '\n';
}

void Logger::Warning(const std::string& message)
{
    _messages.push_back({ LogLevel::Warning, message });
    std::cout << "[Warning] " << message << '\n';
}

void Logger::Error(const std::string& message)
{
    _messages.push_back({ LogLevel::Error, message });
    std::cout << "[Error] " << message << '\n';
}

void Logger::Clear()
{
    _messages.clear();
}
