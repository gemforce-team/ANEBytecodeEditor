#pragma once

#include <exception>
#include <string>

#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#else
#include <source_location>
#endif

class StringException : public std::exception
{
public:
#ifdef __cpp_lib_stacktrace
    StringException(std::string data, std::stacktrace trace = std::stacktrace::current())
    {
        this->data = data + "\nStack trace: " + std::to_string(trace);
    }
#else
    StringException(std::string data, std::source_location trace = std::source_location::current())
    {
        this->data = data + "\nAt: " + trace.file_name() + "(" + std::to_string(trace.line()) +
                     ") " + trace.function_name();
    }
#endif

    const char* what() const noexcept override { return data.c_str(); }

private:
    std::string data;
};
