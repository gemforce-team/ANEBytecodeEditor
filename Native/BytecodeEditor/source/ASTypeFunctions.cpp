#include "ANEFunctions.hpp"
#include "enums/InstanceFlags.hpp"

#define GET_TYPE(type) type& clazz = *reinterpret_cast<type*>(funcData)

#define SUCCEED_VOID()                                                                             \
    FREObject ret;                                                                                 \
    DO_OR_FAIL("Could not create success boolean", FRENewObjectFromBool(true, &ret));              \
    return ret;

namespace
{
    template <typename T, auto accessor>
    FREObject GetTrait(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(2);

        GET_TYPE(T);

        try
        {
            ASASM::Multiname name = ConvertMultiname(argv[0]);

            bool favorSetter = CHECK_OBJECT<FRE_TYPE_BOOLEAN>(argv[1]);

            SmallTrivialVector<const ASASM::Trait*, 2> found;

            for (const auto& trait : std::invoke(accessor, clazz).traits)
            {
                if (trait.name == name)
                {
                    if (!found.emplace_back(&trait))
                    {
                        FAIL("More than two traits matched this multiname. This should never "
                             "happen!");
                    }
                }
            }

            if (found.size() == 1)
            {
                return ConvertTrait(*found[0]);
            }
            else if (found.size() == 2)
            {
                if (favorSetter && found[0]->kind == TraitKind::Getter)
                {
                    if (favorSetter)
                    {
                        return ConvertTrait(*found[1]);
                    }
                    else
                    {
                        return ConvertTrait(*found[0]);
                    }
                }
                else
                {
                    if (favorSetter)
                    {
                        return ConvertTrait(*found[0]);
                    }
                    else
                    {
                        return ConvertTrait(*found[1]);
                    }
                }
            }
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }

        return nullptr;
    }

    template <typename T, auto accessor>
    FREObject SetTrait(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_TYPE(T);

        try
        {
            ASASM::Trait value = ConvertTrait(argv[0]);

            SmallTrivialVector<size_t, 2> found{};

            auto& traits = std::invoke(accessor, clazz).traits;

            for (size_t i = 0; i < traits.size(); i++)
            {
                if (traits[i].name == value.name)
                {
                    if (!found.emplace_back(i))
                    {
                        FAIL("More than two traits matched this multiname. This should never "
                             "happen!");
                    }
                }
            }

            if (found.size() > 0)
            {
                if (found.size() == 2) // Must be both getter and setter
                {
                    if (value.kind == TraitKind::Getter)
                    {
                        if (traits[found[0]].kind == TraitKind::Getter)
                        {
                            traits[found[0]] = std::move(value);
                        }
                        else
                        {
                            traits[found[1]] = std::move(value);
                        }
                    }
                    else if (value.kind == TraitKind::Setter)
                    {
                        if (traits[found[0]].kind == TraitKind::Setter)
                        {
                            traits[found[0]] = std::move(value);
                        }
                        else
                        {
                            traits[found[1]] = std::move(value);
                        }
                    }
                    else
                    {
                        traits.erase(
                            traits.begin() +
                            found[1]); // Erase found[1] because it's the closest to the back
                        traits[found[0]] = std::move(value); // And replace found[0]
                    }
                }
                else
                {
                    TraitKind kind = traits[found[0]].kind;
                    if ((kind == TraitKind::Getter || kind == TraitKind::Setter) &&
                        (value.kind == TraitKind::Setter || value.kind == TraitKind::Getter) &&
                        kind == value.kind)
                    {
                        traits[found[0]] = std::move(value);
                    }
                    else if ((kind == TraitKind::Getter || kind == TraitKind::Setter) &&
                             (value.kind == TraitKind::Setter || value.kind == TraitKind::Getter) &&
                             kind != value.kind)
                    {
                        traits.emplace_back(std::move(value));
                    }
                    else
                    {
                        traits[found[0]] = std::move(value);
                    }
                }
            }
            else
            {
                traits.emplace_back(std::move(value));
            }
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }

        SUCCEED_VOID();
    }

    template <typename T, auto accessor>
    FREObject DeleteTrait(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(2);

        GET_TYPE(T);

        try
        {
            ASASM::Multiname name = ConvertMultiname(argv[0]);

            bool favorSetter = CHECK_OBJECT<FRE_TYPE_BOOLEAN>(argv[1]);

            auto& traits = std::invoke(accessor, clazz).traits;

            SmallTrivialVector<size_t, 2> found;

            for (size_t i = 0; i < traits.size(); i++)
            {
                const auto& trait = traits[i];
                if (trait.name == name)
                {
                    if (!found.emplace_back(i))
                    {
                        FAIL("More than two traits matched this multiname. This should never "
                             "happen!");
                    }
                }
            }

            if (found.size() == 1)
            {
                traits.erase(traits.begin() + found[0]);
                SUCCEED_VOID();
            }
            else if (found.size() == 2)
            {
                if (traits[found[0]].kind == TraitKind::Getter)
                {
                    if (favorSetter)
                    {
                        traits.erase(traits.begin() + found[1]);
                    }
                    else
                    {
                        traits.erase(traits.begin() + found[0]);
                    }
                }
                else
                {
                    if (favorSetter)
                    {
                        traits.erase(traits.begin() + found[0]);
                    }
                    else
                    {
                        traits.erase(traits.begin() + found[1]);
                    }
                }
                SUCCEED_VOID();
            }

            FAIL("Trait not found");
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    template <typename T, auto accessor>
    FREObject GetConstructor(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_TYPE(T);

        try
        {
            return ConvertMethod(*std::invoke(accessor, clazz));
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    template <typename T, auto accessor>
    FREObject SetConstructor(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_TYPE(T);

        try
        {
            std::invoke(accessor, clazz) = ConvertMethod(argv[0]);
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }

        SUCCEED_VOID();
    }
}

namespace ASClass
{
    FREObject GetStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return GetTrait<ASASM::Class, [](ASASM::Class& c) -> ASASM::Class& { return c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return SetTrait<ASASM::Class, [](ASASM::Class& c) -> ASASM::Class& { return c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject DeleteStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return DeleteTrait<ASASM::Class, [](ASASM::Class& c) -> ASASM::Class& { return c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject GetInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return GetTrait<ASASM::Class, [](ASASM::Class& c) -> ASASM::Instance& {
            return c.instance;
        }>(ctx, funcData, argc, argv);
    }

    FREObject SetInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return SetTrait<ASASM::Class, [](ASASM::Class& c) -> ASASM::Instance& {
            return c.instance;
        }>(ctx, funcData, argc, argv);
    }

    FREObject DeleteInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return DeleteTrait<ASASM::Class, [](ASASM::Class& c) -> ASASM::Instance& {
            return c.instance;
        }>(ctx, funcData, argc, argv);
    }

    FREObject GetStaticConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetConstructor<ASASM::Class,
            [](ASASM::Class& c) -> std::shared_ptr<ASASM::Method>& { return c.cinit; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetStaticConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetConstructor<ASASM::Class,
            [](ASASM::Class& c) -> std::shared_ptr<ASASM::Method>& { return c.cinit; }>(
            ctx, funcData, argc, argv);
    }

    FREObject GetConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetConstructor<ASASM::Class,
            [](ASASM::Class& c) -> std::shared_ptr<ASASM::Method>& { return c.instance.iinit; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetConstructor<ASASM::Class,
            [](ASASM::Class& c) -> std::shared_ptr<ASASM::Method>& { return c.instance.iinit; }>(
            ctx, funcData, argc, argv);
    }

    FREObject GetInterfaces(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_TYPE(ASASM::Class);

        try
        {
            FREObject ret;
            DO_OR_FAIL("Couldn't create interface vector",
                ANENewObject("Vector.<com.cff.anebe.ir.ASMultiname>", 0, nullptr, &ret, nullptr));
            DO_OR_FAIL("Couldn't set interface vector size",
                FRESetArrayLength(ret, clazz.instance.interfaces.size()));
            for (size_t i = 0; i < clazz.instance.interfaces.size(); i++)
            {
                DO_OR_FAIL("Couldn't set interface vector entry",
                    FRESetArrayElementAt(ret, i, ConvertMultiname(clazz.instance.interfaces[i])));
            }
            return ret;
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    FREObject SetInterfaces(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_TYPE(ASASM::Class);

        try
        {
            uint32_t arrLen;
            DO_OR_FAIL("Couldn't get interface vector size", FREGetArrayLength(argv[0], &arrLen));
            std::vector<ASASM::Multiname> newMultinames;
            for (size_t i = 0; i < arrLen; i++)
            {
                FREObject mname;
                DO_OR_FAIL("Couldn't get interface vector element",
                    FREGetArrayElementAt(argv[0], i, &mname));
                newMultinames.emplace_back(ConvertMultiname(mname));
            }

            clazz.instance.interfaces = newMultinames;
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }

        SUCCEED_VOID();
    }

    FREObject GetSuperclass(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_TYPE(ASASM::Class);

        try
        {
            return ConvertMultiname(clazz.instance.superName);
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    FREObject SetSuperclass(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_TYPE(ASASM::Class);

        try
        {
            clazz.instance.superName = ConvertMultiname(argv[0]);
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }

        SUCCEED_VOID();
    }

    FREObject GetFlags(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_TYPE(ASASM::Class);

        try
        {
            FREObject ret;
            DO_OR_FAIL("Couldn't build flag vector",
                ANENewObject("Vector.<String>", 0, nullptr, &ret, nullptr));
            for (const auto& flag : InstanceFlagMap.GetEntries())
            {
                if (clazz.instance.flags & uint8_t(flag.second))
                {
                    uint32_t len;
                    DO_OR_FAIL(
                        "Could not get attributes array length", FREGetArrayLength(ret, &len));
                    DO_OR_FAIL(
                        "Could not set attributes array length", FRESetArrayLength(ret, len + 1));
                    DO_OR_FAIL("Could not add to attribute array",
                        FRESetArrayElementAt(ret, len, FREString(flag.first)));
                }
            }

            return ret;
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    FREObject SetFlags(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_TYPE(ASASM::Class);

        try
        {
            uint8_t flags = 0;
            uint32_t vecSize;
            DO_OR_FAIL("Couldn't get flag vector size", FREGetArrayLength(argv[0], &vecSize));
            for (size_t i = 0; i < vecSize; i++)
            {
                FREObject flag;
                DO_OR_FAIL(
                    "Couldn't get flag vector entry", FREGetArrayElementAt(argv[0], i, &flag));
                flags |= uint8_t(InstanceFlagMap.Find(CHECK_OBJECT<FRE_TYPE_STRING>(flag))->get());
            }

            clazz.instance.flags = flags;
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }

        SUCCEED_VOID();
    }

    FREObject GetProtectedNamespace(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_TYPE(ASASM::Class);

        try
        {
            return ConvertNamespace(clazz.instance.protectedNs);
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    FREObject SetProtectedNamespace(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_TYPE(ASASM::Class);

        try
        {
            clazz.instance.protectedNs = ConvertNamespace(argv[0]);
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (std::exception& e)
        {
            FAIL(std::string("Exception: ") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }

        SUCCEED_VOID();
    }

    FREObject ConvertClassHelper(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_TYPE(ASASM::Class);

        classPointerHelper = &clazz;

        return nullptr;
    }
}

namespace ASScript
{
    FREObject GetInitializer(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetConstructor<ASASM::Script,
            [](ASASM::Script& c) -> std::shared_ptr<ASASM::Method>& { return c.sinit; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetInitializer(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetConstructor<ASASM::Script,
            [](ASASM::Script& c) -> std::shared_ptr<ASASM::Method>& { return c.sinit; }>(
            ctx, funcData, argc, argv);
    }

    FREObject GetTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetTrait<ASASM::Script, [](ASASM::Script& c) -> ASASM::Script& { return c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetTrait<ASASM::Script, [](ASASM::Script& c) -> ASASM::Script& { return c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject DeleteTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::DeleteTrait<ASASM::Script, [](ASASM::Script& c) -> ASASM::Script& { return c; }>(
            ctx, funcData, argc, argv);
    }
}