#include "ANEFunctions.hpp"
#include "enums/InstanceFlags.hpp"
#include <string_view>

using namespace std::string_view_literals;

#define GET_TYPE(type)                                                                             \
    std::shared_ptr<type>& clazz = std::get<std::shared_ptr<type>>(                                \
        static_cast<ANEFunctionContext*>(funcData)->objectData->object)
#define GET_EDITOR() BytecodeEditor& editor = *static_cast<ANEFunctionContext*>(funcData)->editor

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
        GET_EDITOR();

        try
        {
            ASASM::Multiname name = editor.ConvertMultiname(argv[0]);

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
                return editor.ConvertTrait(*found[0]);
            }
            else if (found.size() == 2)
            {
                if (favorSetter && found[0]->kind == TraitKind::Getter)
                {
                    if (favorSetter)
                    {
                        return editor.ConvertTrait(*found[1]);
                    }
                    else
                    {
                        return editor.ConvertTrait(*found[0]);
                    }
                }
                else
                {
                    if (favorSetter)
                    {
                        return editor.ConvertTrait(*found[0]);
                    }
                    else
                    {
                        return editor.ConvertTrait(*found[1]);
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
    FREObject ListTraits(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_TYPE(T);
        GET_EDITOR();

        try
        {
            const auto& traits = std::invoke(accessor, clazz).traits;

            FREObject ret;
            DO_OR_FAIL("Couldn't create trait name vector",
                ANENewObject("Vector.<com.cff.anebe.ir.ASMultiname>", 0, nullptr, &ret, nullptr));
            DO_OR_FAIL(
                "Couldn't set trait name vector size", FRESetArrayLength(ret, traits.size()));
            for (size_t i = 0; i < traits.size(); i++)
            {
                DO_OR_FAIL("Couldn't set trait name vector entry",
                    FRESetArrayElementAt(ret, i, editor.ConvertMultiname(traits[i].name)));
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

    template <typename T, auto accessor>
    FREObject SetTrait(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_TYPE(T);
        GET_EDITOR();

        try
        {
            ASASM::Trait value = editor.ConvertTrait(argv[0]);

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
        GET_EDITOR();

        try
        {
            ASASM::Multiname name = editor.ConvertMultiname(argv[0]);

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
        GET_EDITOR();

        try
        {
            return editor.ConvertMethod(*std::invoke(accessor, clazz));
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
        GET_EDITOR();

        try
        {
            std::invoke(accessor, clazz) = editor.ConvertMethod(argv[0]);
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
        return GetTrait<ASASM::Class, [](const std::shared_ptr<ASASM::Class>& c) -> ASASM::Class& {
            return *c;
        }>(ctx, funcData, argc, argv);
    }

    FREObject ListStaticTraits(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ListTraits<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> ASASM::Class& { return *c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return SetTrait<ASASM::Class, [](const std::shared_ptr<ASASM::Class>& c) -> ASASM::Class& {
            return *c;
        }>(ctx, funcData, argc, argv);
    }

    FREObject DeleteStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return DeleteTrait<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> ASASM::Class& { return *c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject GetInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return GetTrait<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> ASASM::Instance& { return c->instance; }>(
            ctx, funcData, argc, argv);
    }

    FREObject ListInstanceTraits(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ListTraits<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> ASASM::Instance& { return c->instance; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return SetTrait<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> ASASM::Instance& { return c->instance; }>(
            ctx, funcData, argc, argv);
    }

    FREObject DeleteInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return DeleteTrait<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> ASASM::Instance& { return c->instance; }>(
            ctx, funcData, argc, argv);
    }

    FREObject GetStaticConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetConstructor<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> std::shared_ptr<ASASM::Method>&
            { return c->cinit; }>(ctx, funcData, argc, argv);
    }

    FREObject SetStaticConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetConstructor<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> std::shared_ptr<ASASM::Method>&
            { return c->cinit; }>(ctx, funcData, argc, argv);
    }

    FREObject GetConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetConstructor<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> std::shared_ptr<ASASM::Method>&
            { return c->instance.iinit; }>(ctx, funcData, argc, argv);
    }

    FREObject SetConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetConstructor<ASASM::Class,
            [](const std::shared_ptr<ASASM::Class>& c) -> std::shared_ptr<ASASM::Method>&
            { return c->instance.iinit; }>(ctx, funcData, argc, argv);
    }

    FREObject GetInterfaces(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_TYPE(ASASM::Class);
        GET_EDITOR();

        try
        {
            FREObject ret;
            DO_OR_FAIL("Couldn't create interface vector",
                ANENewObject("Vector.<com.cff.anebe.ir.ASMultiname>", 0, nullptr, &ret, nullptr));
            DO_OR_FAIL("Couldn't set interface vector size",
                FRESetArrayLength(ret, clazz->instance.interfaces.size()));
            for (size_t i = 0; i < clazz->instance.interfaces.size(); i++)
            {
                DO_OR_FAIL("Couldn't set interface vector entry",
                    FRESetArrayElementAt(
                        ret, i, editor.ConvertMultiname(clazz->instance.interfaces[i])));
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
        GET_EDITOR();

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
                newMultinames.emplace_back(editor.ConvertMultiname(mname));
            }

            clazz->instance.interfaces = newMultinames;
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
        GET_EDITOR();

        try
        {
            return editor.ConvertMultiname(clazz->instance.superName);
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
        GET_EDITOR();

        try
        {
            clazz->instance.superName = editor.ConvertMultiname(argv[0]);
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

    FREObject GetInstanceName(FREContext, void* funcData, uint32_t argc, FREObject[])
    {
        CHECK_ARGC(0);

        GET_TYPE(ASASM::Class);
        GET_EDITOR();

        try
        {
            return editor.ConvertMultiname(clazz->instance.name);
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

    FREObject SetInstanceName(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(1);

        GET_TYPE(ASASM::Class);
        GET_EDITOR();

        try
        {
            clazz->instance.name = editor.ConvertMultiname(argv[0]);
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
                if (clazz->instance.flags & uint8_t(flag.second))
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

            clazz->instance.flags = flags;
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
        GET_EDITOR();

        try
        {
            return editor.ConvertNamespace(clazz->instance.protectedNs);
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
        GET_EDITOR();

        try
        {
            clazz->instance.protectedNs = editor.ConvertNamespace(argv[0]);
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
        GET_EDITOR();

        nextObjectContext = ANEFunctionContext{
            editor.shared_from_this(), nullptr, ANEFunctionContext::ObjectData{clazz}};

        return nullptr;
    }
}

namespace ASScript
{
    FREObject GetInitializer(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetConstructor<ASASM::Script,
            [](const std::shared_ptr<ASASM::Script>& c) -> std::shared_ptr<ASASM::Method>&
            { return c->sinit; }>(ctx, funcData, argc, argv);
    }

    FREObject SetInitializer(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetConstructor<ASASM::Script,
            [](const std::shared_ptr<ASASM::Script>& c) -> std::shared_ptr<ASASM::Method>&
            { return c->sinit; }>(ctx, funcData, argc, argv);
    }

    FREObject GetTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::GetTrait<ASASM::Script,
            [](const std::shared_ptr<ASASM::Script>& c) -> ASASM::Script& { return *c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject ListTraits(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::ListTraits<ASASM::Script,
            [](const std::shared_ptr<ASASM::Script>& c) -> ASASM::Script& { return *c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject SetTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::SetTrait<ASASM::Script,
            [](const std::shared_ptr<ASASM::Script>& c) -> ASASM::Script& { return *c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject DeleteTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
    {
        return ::DeleteTrait<ASASM::Script,
            [](const std::shared_ptr<ASASM::Script>& c) -> ASASM::Script& { return *c; }>(
            ctx, funcData, argc, argv);
    }

    FREObject CreateClass(FREContext, void* funcData, uint32_t argc, FREObject argv[])
    {
        CHECK_ARGC(3);

        GET_TYPE(ASASM::Script);
        GET_EDITOR();

        try
        {
            ASASM::Trait& newTrait = clazz->traits.emplace_back();
            newTrait.kind          = TraitKind::Class;
            newTrait.name          = editor.ConvertMultiname(argv[0]);
            newTrait.vClass({0,
                std::shared_ptr<ASASM::Class>(new ASASM::Class(editor.ConvertMethod(argv[1]), {},
                    ASASM::Instance{
                        .name = newTrait.name, .iinit = editor.ConvertMethod(argv[2])}))});

            return editor.ConvertClass(newTrait.vClass().vclass);
        }
        catch (std::nullptr_t)
        {
            FAIL("nullptr caught");
        }
        catch (FREObject o)
        {
            return o;
        }
        catch (std::exception& e)
        {
            FAIL(e.what());
        }
        catch (...)
        {
            FAIL("Some weird thing caught");
        }
    }
}
