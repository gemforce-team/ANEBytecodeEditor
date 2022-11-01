#pragma once

#include "ASASM/Method.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Namespace.hpp"
#include "ASASM/Trait.hpp"

#include <memory>
#include <stdint.h>
#include <vector>

namespace ASASM
{
    struct Instance
    {
        Multiname name;
        Multiname superName;
        uint8_t flags = 0;
        Namespace protectedNs;
        std::vector<Multiname> interfaces;
        std::shared_ptr<Method> iinit;
        std::vector<Trait> traits;

        auto operator<=>(const Instance&) const noexcept = default;
        bool operator==(const Instance&) const noexcept  = default;
    };
}
