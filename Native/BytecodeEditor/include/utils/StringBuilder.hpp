#pragma once

#include <sstream>

class StringBuilder : public std::stringstream
{
public:
    bool indented;
    int indent;
    std::string linePrefix;

    template <typename T>
    void write(T v)
    {
        checkIndent();
        *this << std::forward<T>(v);
    }

    void newLine()
    {
        *this << '\n';
        indented = false;
    }

    void noIndent() { indented = true; }

    void checkIndent()
    {
        if (!indented)
        {
            // for (int i = 0; i < indent; i++)
            //  *this << ' ';
            indented = true;
            if (!linePrefix.empty())
                *this << linePrefix;
        }
    }
};
