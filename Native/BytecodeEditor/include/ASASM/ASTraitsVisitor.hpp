#pragma once

#include "ASASM/ASProgram.hpp"

namespace ASASM
{
    class ASTraitsVisitor
    {
    protected:
        const ASASM::ASProgram& as;

    public:
        explicit ASTraitsVisitor(const ASProgram& as) noexcept : as(as) {}
        virtual ~ASTraitsVisitor() = default;

        virtual void run()
        {
            for (const auto& v : as.scripts)
            {
                visitTraits(v.traits);
            }
        }

        void visitTraits(const std::vector<Trait>& traits)
        {
            for (const auto& trait : traits)
            {
                visitTrait(trait);
            }
        }

        virtual void visitTrait(const Trait& trait)
        {
            switch (trait.kind)
            {
                case TraitKind::Slot:
                case TraitKind::Const:
                    break;
                case TraitKind::Class:
                    visitTraits(trait.vClass().vclass->traits);
                    visitTraits(trait.vClass().vclass->instance.traits);
                    break;
                case TraitKind::Function:
                    if (trait.vFunction().vfunction->vbody)
                        visitTraits(trait.vFunction().vfunction->vbody->traits);
                    break;
                case TraitKind::Method:
                case TraitKind::Getter:
                case TraitKind::Setter:
                    if (trait.vMethod().vmethod->vbody)
                        visitTraits(trait.vMethod().vmethod->vbody->traits);
                    break;
                default:
                    throw StringException("Unknown trait kind");
            }
        }
    };
}
