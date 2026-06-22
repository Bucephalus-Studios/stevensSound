/**
 * @file ErrorHandler.hpp
 * @brief Thread-safe error handling system for stevensSound library
 */

#pragma once

#include <string>
#include <functional>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace stevensSound
{

/**
 * @brief Error severity levels
 */
enum class ErrorLevel
{
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @brief Error information structure
 */
struct ErrorInfo
{
    ErrorLevel level;
    std::string message;
    std::string function;
    std::string timestamp;

    ErrorInfo() : level(ErrorLevel::INFO), message(""), function(""), timestamp("") {}

    ErrorInfo(ErrorLevel lvl, const std::string& msg, const std::string& func)
        : level(lvl), message(msg), function(func)
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        timestamp = ss.str();
    }

    std::string toString() const
    {
        std::string levelStr;
        switch(level)
        {
            case ErrorLevel::INFO:     levelStr = "INFO"; break;
            case ErrorLevel::WARNING:  levelStr = "WARNING"; break;
            case ErrorLevel::ERROR:    levelStr = "ERROR"; break;
            case ErrorLevel::CRITICAL: levelStr = "CRITICAL"; break;
        }
        return "[" + timestamp + "] [" + levelStr + "] " + function + "(): " + message;
    }
};

/**
 * @brief Thread-safe error handler class
 */
class ErrorHandler
{
private:
    static thread_local ErrorInfo lastError;
    static std::function<void(const ErrorInfo&)> customHandler;
    static std::mutex handlerMutex;
    static bool enableLogging;

public:
    static void setErrorHandler(std::function<void(const ErrorInfo&)> handler)
    {
        std::lock_guard<std::mutex> lock(handlerMutex);
        customHandler = handler;
    }

    static void setLogging(bool enable)
    {
        enableLogging = enable;
    }

    static void setError(ErrorLevel level, const std::string& message,
                        const std::string& function)
    {
        ErrorInfo error(level, message, function);
        lastError = error;

        {
            std::lock_guard<std::mutex> lock(handlerMutex);
            if (customHandler)
            {
                customHandler(error);
            }
        }

        if (enableLogging)
        {
            std::cout << error.toString() << std::endl;
        }
    }

    static ErrorInfo getLastError()
    {
        return lastError;
    }

    static std::string getLastErrorMessage()
    {
        return lastError.message;
    }

    static void clearError()
    {
        lastError = ErrorInfo();
    }

    static bool hasError()
    {
        return !lastError.message.empty();
    }
};

// Static member definitions
inline thread_local ErrorInfo ErrorHandler::lastError;
inline std::function<void(const ErrorInfo&)> ErrorHandler::customHandler = nullptr;
inline std::mutex ErrorHandler::handlerMutex;
inline bool ErrorHandler::enableLogging = false;

} // namespace stevensSound
