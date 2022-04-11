#pragma once

#include "ABC/Label.hpp"
#include "ASASM/Multiname.hpp"

namespace ASASM
{
    struct Exception
    {
        ABC::Label from, to, target;
        Multiname excType;
        Multiname varName;

        bool operator==(const Exception&) const = default;
        bool operator<(const Exception& other) const
        {
            if (from < other.from)
            {
                return true;
            }
            else if (from == other.from)
            {
                if (to < other.to)
                {
                    return true;
                }
                else if (to == other.to)
                {
                    if (target < other.target)
                    {
                        return true;
                    }
                    else if (target == other.target)
                    {
                        if (excType < other.excType)
                        {
                            return true;
                        }
                        else if (excType == other.excType)
                        {
                            return varName == other.varName;
                        }
                    }
                }
            }

            return false;
        }
    };
}
