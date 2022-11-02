#include "ANEFunctions.hpp"
#include "enums/InstanceFlags.hpp"

#define GET_CLASS() ASASM::Class& clazz = *reinterpret_cast<ASASM::Class*>(funcData)

#define SUCCEED_VOID()                                                                             \
    FREObject ret;                                                                                 \
    DO_OR_FAIL("Could not create success boolean", FRENewObjectFromBool(true, &ret));              \
    return ret;

namespace
{
    template <auto accessor>
    FREObject GetTrait(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(2);

        GET_CLASS();

        ASASM::Multiname name = ConvertMultiname(argv[0]);

        uint32_t whichIfMultiple = CHECK_OBJECT<FRE_TYPE_NUMBER, uint32_t>(argv[1]);

        uint32_t currentIfMultiple = 0;

        try
        {
            for (const auto& trait : std::invoke(accessor, clazz).traits)
            {
                if (trait.name == name)
                {
                    if (currentIfMultiple == whichIfMultiple)
                    {
                        return ConvertTrait(trait);
                    }
                    currentIfMultiple++;
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
            FAIL(std::string("Exception") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }

        return nullptr;
    }

    template <auto accessor>
    FREObject SetTrait(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_CLASS();

        try
        {
            ASASM::Trait value = ConvertTrait(argv[0]);

            bool hasSet = false;
            for (size_t i = 0; i < std::invoke(accessor, clazz).traits.size(); i++)
            {
                auto& trait = std::invoke(accessor, clazz).traits[i];
                if (trait.name == value.name)
                {
                    trait  = std::move(value);
                    hasSet = true;
                    break;
                }
            }

            if (!hasSet)
            {
                std::invoke(accessor, clazz).traits.emplace_back(std::move(ConvertTrait(argv[0])));
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
            FAIL(std::string("Exception") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }

        SUCCEED_VOID();
    }

    template <auto accessor>
    FREObject DeleteTrait(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(2);

        GET_CLASS();

        ASASM::Multiname name = ConvertMultiname(argv[0]);

        try
        {
            uint32_t whichIfMultiple = CHECK_OBJECT<FRE_TYPE_NUMBER, uint32_t>(argv[1]);

            uint32_t currentIfMultiple = 0;

            for (size_t i = 0; i < std::invoke(accessor, clazz).traits.size(); i++)
            {
                const auto& trait = std::invoke(accessor, clazz).traits[i];
                if (trait.name == name)
                {
                    if (currentIfMultiple == whichIfMultiple)
                    {
                        std::invoke(accessor, clazz)
                            .traits.erase(std::invoke(accessor, clazz).traits.begin() + i);
                        SUCCEED_VOID();
                    }
                    currentIfMultiple++;
                }
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
            FAIL(std::string("Exception") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    template <auto accessor>
    FREObject GetConstructor(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    template <auto accessor>
    FREObject SetConstructor(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
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
        return GetTrait<[](ASASM::Class& c) -> ASASM::Class& { return c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return SetTrait<[](ASASM::Class& c) -> ASASM::Class& { return c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject DeleteStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return DeleteTrait<[](ASASM::Class& c) -> ASASM::Class& { return c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject GetInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return GetTrait<[](ASASM::Class& c) -> ASASM::Instance& { return c.instance; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return SetTrait<[](ASASM::Class& c) -> ASASM::Instance& { return c.instance; }>(
            ctx, funcData, argc, argv);
    }

    FREObject DeleteInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return DeleteTrait<[](ASASM::Class& c) -> ASASM::Instance& { return c.instance; }>(
            ctx, funcData, argc, argv);
    }

    FREObject GetStaticConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetConstructor<[](ASASM::Class& c) -> std::shared_ptr<ASASM::Method>&
            { return c.cinit; }>(ctx, funcData, argc, argv);
    }

    FREObject SetStaticConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetConstructor<[](ASASM::Class& c) -> std::shared_ptr<ASASM::Method>&
            { return c.cinit; }>(ctx, funcData, argc, argv);
    }

    FREObject GetConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetConstructor<[](ASASM::Class& c) -> std::shared_ptr<ASASM::Method>&
            { return c.instance.iinit; }>(ctx, funcData, argc, argv);
    }

    FREObject SetConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetConstructor<[](ASASM::Class& c) -> std::shared_ptr<ASASM::Method>&
            { return c.instance.iinit; }>(ctx, funcData, argc, argv);
    }

    FREObject GetInterfaces(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    FREObject SetInterfaces(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
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

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    FREObject SetSuperclass(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
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

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    FREObject SetFlags(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
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

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }

    FREObject SetProtectedNamespace(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_CLASS();

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
            FAIL(std::string("Exception") + e.what());
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

        GET_CLASS();

        classPointerHelper = &clazz;

        return nullptr;
    }
}
