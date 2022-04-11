#pragma once

#include "ABC/Class.hpp"
#include "ABC/Instance.hpp"
#include "ABC/Metadata.hpp"
#include "ABC/MethodBody.hpp"
#include "ABC/MethodInfo.hpp"
#include "ABC/Multiname.hpp"
#include "ABC/Namespace.hpp"
#include "ABC/Script.hpp"
#include <stdint.h>
#include <string>

namespace ABC
{
    struct ABCFile
    {
        uint16_t minorVersion, majorVersion;
        std::vector<int64_t> ints;
        std::vector<uint64_t> uints;
        std::vector<double> doubles;
        std::vector<std::string> strings;
        std::vector<Namespace> namespaces;
        std::vector<std::vector<int32_t>> namespaceSets;
        std::vector<Multiname> multinames;

        std::vector<MethodInfo> methods;
        std::vector<Metadata> metadata;
        std::vector<Instance> instances;
        std::vector<Class> classes;
        std::vector<Script> scripts;
        std::vector<MethodBody> bodies;

        static constexpr int64_t NULL_INT   = INT64_MAX;
        static constexpr uint64_t NULL_UINT = UINT64_MAX;
        static constexpr double NULL_DOUBLE = std::numeric_limits<double>::quiet_NaN();

        static constexpr uint64_t MAX_UINT = (((uint64_t)1) << 36) - 1;
        static constexpr int64_t MAX_INT   = MAX_UINT / 2;
        static constexpr int64_t MIN_INT   = -MAX_INT - 1;

        ABCFile()
        {
            majorVersion = 46;
            minorVersion = 16;

            ints    = {NULL_INT};
            uints   = {NULL_UINT};
            doubles = {NULL_DOUBLE};

            strings       = {""};
            namespaces    = {{}};
            namespaceSets = {{}};
            multinames    = {{}};
        }
    };
}
