/**
 * @file s_errorHandler.h
 * @brief Thread-safe error handling system for stevensSound library
 * @version 1.0
 * @date 2025-11-14
 */

#ifndef STEVENSSOUND_ERROR_HANDLER_H
#define STEVENSSOUND_ERROR_HANDLER_H

#include <string>
#include <functional>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace stevensSound
{
    /**
     * @brief Error severity levels
     */
    enum class ErrorLevel
    {
        INFO,       // Informational messages
        WARNING,    // Non-critical issues
        ERROR,      // Errors that prevent operations
        CRITICAL    // Critical errors that may cause instability
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
            // Generate timestamp
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

            return "[" + timestamp + "] [" + levelStr + "] " +
                   function + "(): " + message;
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
        /**
         * @brief Set a custom error handler callback
         * @param handler Function to call when errors occur (nullptr to disable)
         */
        static void setErrorHandler(std::function<void(const ErrorInfo&)> handler)
        {
            std::lock_guard<std::mutex> lock(handlerMutex);
            customHandler = handler;
        }

        /**
         * @brief Enable or disable error logging to stdout
         * @param enable If true, errors are printed to stdout
         */
        static void setLogging(bool enable)
        {
            enableLogging = enable;
        }

        /**
         * @brief Record an error
         * @param level Error severity level
         * @param message Error message
         * @param function Function name where error occurred
         */
        static void setError(ErrorLevel level, const std::string& message,
                           const std::string& function)
        {
            ErrorInfo error(level, message, function);
            lastError = error;

            // Call custom handler if set
            {
                std::lock_guard<std::mutex> lock(handlerMutex);
                if (customHandler)
                {
                    customHandler(error);
                }
            }

            // Log to stdout if enabled
            if (enableLogging)
            {
                std::cout << error.toString() << std::endl;
            }
        }

        /**
         * @brief Get the last error that occurred on this thread
         * @return ErrorInfo structure with error details
         */
        static ErrorInfo getLastError()
        {
            return lastError;
        }

        /**
         * @brief Get the last error message as a string
         * @return Error message string
         */
        static std::string getLastErrorMessage()
        {
            return lastError.message;
        }

        /**
         * @brief Clear the last error
         */
        static void clearError()
        {
            lastError = ErrorInfo();
        }

        /**
         * @brief Check if there is an error set
         * @return True if an error has been recorded
         */
        static bool hasError()
        {
            return !lastError.message.empty();
        }
    };

    // Static member initialization
    thread_local ErrorInfo ErrorHandler::lastError;
    std::function<void(const ErrorInfo&)> ErrorHandler::customHandler = nullptr;
    std::mutex ErrorHandler::handlerMutex;
    bool ErrorHandler::enableLogging = false;

} // namespace stevensSound

#endif // STEVENSSOUND_ERROR_HANDLER_H
