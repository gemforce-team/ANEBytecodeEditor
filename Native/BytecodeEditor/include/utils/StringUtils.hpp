#pragma once

#include <string>
#include <vector>

namespace StringUtils
{
    std::vector<std::string> split(std::string_view s, std::string_view delimiter)
    {
        std::vector<std::string> ret;
        if (delimiter.size() == 0)
        {
            ret.reserve(s.size());
            for (size_t i = 0; i < s.size(); i++)
            {
                ret.emplace_back(s.substr(i, 1));
            }
            return ret;
        }
        size_t found = s.find(delimiter);
        while (found != std::string::npos)
        {
            ret.emplace_back(s.substr(0, found));
            s     = s.substr(found + delimiter.size());
            found = s.find(delimiter);
        }
        ret.emplace_back(s);

        return ret;
    }

    std::string join(const std::string_view* s, size_t len, std::string_view joiner)
    {
        if (len == 0)
        {
            return "";
        }

        size_t sizeNeeded = s[0].size();
        for (size_t i = 1; i < len; i++)
        {
            sizeNeeded += s[i].size() + joiner.size();
        }

        std::string ret;
        ret.reserve(sizeNeeded);

        ret += s[0];
        for (size_t i = 1; i < len; i++)
        {
            ret += joiner;
            ret += s[i];
        }
        return ret;
    }

    std::string join(const std::string* s, size_t len, std::string_view joiner)
    {
        if (len == 0)
        {
            return "";
        }

        size_t sizeNeeded = s[0].size();
        for (size_t i = 1; i < len; i++)
        {
            sizeNeeded += s[i].size() + joiner.size();
        }

        std::string ret;
        ret.reserve(sizeNeeded);

        ret += s[0];
        for (size_t i = 1; i < len; i++)
        {
            ret += joiner;
            ret += s[i];
        }
        return ret;
    }

    std::string join(const std::vector<std::string_view>& s, std::string_view joiner)
    {
        return join(s.data(), s.size(), joiner);
    }

    std::string join(const std::vector<std::string>& s, std::string_view joiner)
    {
        return join(s.data(), s.size(), joiner);
    }
}
