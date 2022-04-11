#pragma once

#include "ABC/ABCFile.hpp"
#include "ASASM/Class.hpp"
#include "ASASM/Method.hpp"
#include "ASASM/Script.hpp"
#include "utils/StringException.hpp"

#include <memory>
#include <stdint.h>
#include <vector>

class Assembler;

namespace ASASM
{
    class ASProgram
    {
        friend class ::Assembler;

    private:
        ASProgram() : minorVersion(16), majorVersion(46) {}

    public:
        uint16_t minorVersion, majorVersion;

        std::vector<Script> scripts;
        // Pointers to the below
        std::vector<std::shared_ptr<Class>> orphanClasses;
        std::vector<std::shared_ptr<Method>> orphanMethods;

        static ASProgram fromABC(const ABC::ABCFile& abc);
        ABC::ABCFile toABC();
    };
}
