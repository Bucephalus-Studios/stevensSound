/**
 * @file test_error_handler.cpp
 * @brief Tests for the error handling system
 */

#include <gtest/gtest.h>
#include "../stevensSound.hpp"

using namespace stevensSound;

class ErrorHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ErrorHandler::clearError();
        ErrorHandler::setLogging(false);
    }

    void TearDown() override
    {
        ErrorHandler::clearError();
        ErrorHandler::setErrorHandler(nullptr);
    }
};

TEST_F(ErrorHandlerTest, SetAndGetError)
{
    ErrorHandler::setError(ErrorLevel::ERROR, "Test error", "TestFunction");

    EXPECT_TRUE(ErrorHandler::hasError());
    EXPECT_EQ(ErrorHandler::getLastErrorMessage(), "Test error");

    auto errorInfo = ErrorHandler::getLastError();
    EXPECT_EQ(errorInfo.level, ErrorLevel::ERROR);
    EXPECT_EQ(errorInfo.message, "Test error");
    EXPECT_EQ(errorInfo.function, "TestFunction");
}

TEST_F(ErrorHandlerTest, ClearError)
{
    ErrorHandler::setError(ErrorLevel::ERROR, "Test error", "TestFunction");
    EXPECT_TRUE(ErrorHandler::hasError());

    ErrorHandler::clearError();
    EXPECT_FALSE(ErrorHandler::hasError());
    EXPECT_EQ(ErrorHandler::getLastErrorMessage(), "");
}

TEST_F(ErrorHandlerTest, CustomErrorHandler)
{
    bool handlerCalled = false;
    std::string capturedMessage;

    auto customHandler = [&](const ErrorInfo& error) {
        handlerCalled = true;
        capturedMessage = error.message;
    };

    ErrorHandler::setErrorHandler(customHandler);
    ErrorHandler::setError(ErrorLevel::WARNING, "Custom handler test", "TestFunction");

    EXPECT_TRUE(handlerCalled);
    EXPECT_EQ(capturedMessage, "Custom handler test");
}

TEST_F(ErrorHandlerTest, ErrorLevels)
{
    ErrorHandler::setError(ErrorLevel::INFO, "Info message", "TestFunction");
    EXPECT_EQ(ErrorHandler::getLastError().level, ErrorLevel::INFO);

    ErrorHandler::setError(ErrorLevel::WARNING, "Warning message", "TestFunction");
    EXPECT_EQ(ErrorHandler::getLastError().level, ErrorLevel::WARNING);

    ErrorHandler::setError(ErrorLevel::ERROR, "Error message", "TestFunction");
    EXPECT_EQ(ErrorHandler::getLastError().level, ErrorLevel::ERROR);

    ErrorHandler::setError(ErrorLevel::CRITICAL, "Critical message", "TestFunction");
    EXPECT_EQ(ErrorHandler::getLastError().level, ErrorLevel::CRITICAL);
}

TEST_F(ErrorHandlerTest, ErrorToString)
{
    ErrorHandler::setError(ErrorLevel::ERROR, "Test error", "TestFunction");
    auto errorInfo = ErrorHandler::getLastError();

    std::string errorStr = errorInfo.toString();
    EXPECT_NE(errorStr.find("[ERROR]"), std::string::npos);
    EXPECT_NE(errorStr.find("TestFunction"), std::string::npos);
    EXPECT_NE(errorStr.find("Test error"), std::string::npos);
}
