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

        bool operator==(const Instance& other) const = default;
        bool operator<(const Instance& other) const
        {
            if (name < other.name)
            {
                return true;
            }
            else if (name == other.name)
            {
                if (superName < other.superName)
                {
                    return true;
                }
                else if (superName == other.superName)
                {
                    if (flags < other.flags)
                    {
                        return true;
                    }
                    else if (flags == other.flags)
                    {
                        if (protectedNs < other.protectedNs)
                        {
                            return true;
                        }
                        else if (protectedNs == other.protectedNs)
                        {
                            if (interfaces < other.interfaces)
                            {
                                return true;
                            }
                            else if (interfaces == other.interfaces)
                            {
                                if (iinit < other.iinit)
                                {
                                    return true;
                                }
                                else if (iinit == other.iinit)
                                {
                                    return traits < other.traits;
                                }
                            }
                        }
                    }
                }
            }
            return false;
        }
    };
}
