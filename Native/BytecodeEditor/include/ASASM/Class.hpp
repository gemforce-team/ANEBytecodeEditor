#pragma once

#include "ASASM/Instance.hpp"
#include "ASASM/Method.hpp"
#include "ASASM/Trait.hpp"

#include <memory>
#include <vector>

namespace ASASM
{
    struct Class
    {
        std::shared_ptr<Method> cinit;
        std::vector<Trait> traits;
        Instance instance;

        bool operator==(const Class& other) const = default;
        bool operator<(const Class& other) const
        {
            if (cinit < other.cinit)
            {
                return true;
            }
            else if (cinit == other.cinit)
            {
                if (traits < other.traits)
                {
                    return instance < other.instance;
                }
            }
            return false;
        }
    };
}
