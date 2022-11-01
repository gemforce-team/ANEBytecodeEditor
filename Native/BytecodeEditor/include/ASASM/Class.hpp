#pragma once

#include "ASASM/Instance.hpp"
#include "ASASM/Method.hpp"
#include "ASASM/Trait.hpp"

#include <memory>
#include <vector>

namespace ASASM
{
    struct Class : public std::enable_shared_from_this<Class>
    {
        std::shared_ptr<Method> cinit;
        std::vector<Trait> traits;
        Instance instance;

        Class(std::shared_ptr<Method>&& cinit, std::vector<Trait>&& traits, Instance&& instance)
            : cinit(cinit), traits(traits), instance(instance)
        {
        }

        Class() = default;

        auto operator<=>(const Class&) const noexcept = default;
        bool operator==(const Class&) const noexcept  = default;
    };
}
