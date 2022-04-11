#pragma once

#include <exception>
#include <string>

class StringException : public std::exception
{
public:
    StringException(std::string data) : data(data) {}

    const char* what() const noexcept override { return data.c_str(); }

private:
    std::string data;
};
