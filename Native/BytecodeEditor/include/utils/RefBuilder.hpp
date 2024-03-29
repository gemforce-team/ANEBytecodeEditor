#pragma once

#include "ASASM/ASTraitsVisitor.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Namespace.hpp"
#include "enums/ABCType.hpp"
#include "utils/StringUtils.hpp"

#include <algorithm>
#include <cassert>
#include <ctype.h>
#include <list>
#include <stdio.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class RefBuilder : public ASASM::ASTraitsVisitor
{
private:
    static bool nsSimilar(const ASASM::Namespace& ns1, const ASASM::Namespace& ns2)
    {
        if (ns1.kind == ABCType::PrivateNamespace || ns2.kind == ABCType::PrivateNamespace)
        {
            return ns1.kind == ns2.kind && ns1.id == ns2.id;
        }
        // ignore ns kind in other cases
        return ns1.name == ns2.name;
    }

    std::list<std::unordered_set<ASASM::Namespace>> homonymData;
    std::array<std::unordered_map<std::optional<std::string>,
                   std::reference_wrapper<std::unordered_set<ASASM::Namespace>>>,
        ABCTypeMap.GetEntries().size()>
        homonyms;

#ifndef NDEBUG
    bool homonymsBuilt = false;
#endif

public:
    bool hasHomonyms(const ASASM::Namespace& ns) const
    {
        if (auto found = homonyms[(uint8_t)ns.kind].find(ns.name);
            found != homonyms[(uint8_t)ns.kind].end())
        {
            return found->second.get().size() > 1;
        }
        else
        {
            return false;
        }
    }

    void addHomonym(const ASASM::Namespace& ns)
    {
        assert(!homonymsBuilt);
        if (auto found = homonyms[(uint8_t)ns.kind].find(ns.name);
            found != homonyms[(uint8_t)ns.kind].end())
        {
            found->second.get().emplace(ns);
        }
        else
        {
            homonyms[(uint8_t)ns.kind].emplace(ns.name, homonymData.emplace_back());
            homonymData.back().emplace(ns);
        }
    }

    std::optional<std::reference_wrapper<const std::unordered_set<ASASM::Namespace>>> getHomonyms(
        const ASASM::Namespace& ns) const
    {
        if (auto found = homonyms[(uint8_t)ns.kind].find(ns.name);
            found != homonyms[(uint8_t)ns.kind].end())
        {
            return found->second;
        }
        return std::nullopt;
    }

    /// Represents a link in a "context chain", which represents the context in which an object
    /// is encountered. The common root of contexts is used to build ref strings, namespace
    /// labels, file paths etc. Expansion is recursive and delayed until decompilation (an
    /// object may be encountered inside a private namespace, all contexts of which are unknown
    /// until the whole program is scanned).
    struct ContextItem
    {
        friend class ::RefBuilder;
        enum class Type
        {
            Multiname, /// An ASProgram.Multiname (may be a private namespace, etc.)
            String,    /// Fixed string
            Group      /// Multiple ContextItem[]s (which are expanded and the common root is taken
                       /// as the result)
        };

        struct StrData
        {
            std::string str;
            bool filenameSuffix;
            auto operator<=>(const StrData&) const noexcept = default;
            bool operator==(const StrData&) const noexcept  = default;
        };

        struct GroupData
        {
            std::vector<ContextItem> group;
            std::string groupFallback;
            auto operator<=>(const GroupData&) const noexcept = default;
            bool operator==(const GroupData&) const noexcept  = default;
        };

        struct Segment
        {
            char delim;
            std::string str;
            auto operator<=>(const Segment&) const noexcept = default;
            bool operator==(const Segment&) const noexcept  = default;
        };

    private:
        mutable bool expanding = false;
        std::variant<ASASM::Multiname, StrData, GroupData> data;

    public:
        Type type;

        std::vector<ContextItem> reduceGroup(const RefBuilder& refs) const
        {
            if (type != Type::Group)
            {
                throw StringException("Type should be group in reduceGroup");
            }

            std::vector<std::vector<ContextItem>> contexts;
            for (const auto& ctx : std::get<GroupData>(data).group)
            {
                contexts.emplace_back(ContextItem::expand(refs, {ctx}));
            }

            std::vector<ContextItem> ctx;
            if (!contexts.empty())
            {
                ctx = contexts[0];
                for (size_t i = 1; i < contexts.size(); i++)
                {
                    ctx = contextRoot(ctx, contexts[i]);
                }
            }
            if (ctx.empty())
            {
                ctx = {ContextItem(std::get<GroupData>(data).groupFallback)};
            }
            return ctx;
        }

        std::vector<Segment> toSegments(const RefBuilder& refs, bool filename) const
        {
            switch (type)
            {
                case Type::Multiname:
                {
                    const ASASM::Multiname& multiname = std::get<ASASM::Multiname>(data);

                    if (multiname.kind != ABCType::QName)
                    {
                        throw StringException("Multiname kind in toSegments should be QName");
                    }

                    const auto& ns     = multiname.qname().ns;
                    const auto& nsName = ns.name;

                    // if (refs.hasHomonyms(ns))
                    //     nsName += "#" + std::to_string(ns.id);
                    if (nsName)
                    {
                        if (multiname.qname().name)
                        {
                            return {Segment('/', *nsName),
                                Segment(filename ? '/' : ':', *multiname.qname().name)};
                        }
                        else
                        {
                            return {Segment('/', *nsName)};
                        }
                    }
                    else
                    {
                        if (multiname.qname().name)
                        {
                            return {Segment('/', *multiname.qname().name)};
                        }
                        else
                        {
                            throw StringException("Unreachable area in toSegments");
                        }
                    }
                }
                case Type::String:
                    return {
                        Segment((filename && std::get<StrData>(data).filenameSuffix) ? '.' : '/',
                            std::get<StrData>(data).str)};
                case Type::Group:
                {
                    std::vector<Segment> segments;
                    for (const auto& ctx : reduceGroup(refs))
                    {
                        std::vector<Segment> tmp = ctx.toSegments(refs, filename);
                        segments.insert(segments.end(), tmp.begin(), tmp.end());
                    }
                    return segments;
                }
                default:
                    throw StringException("Unreachable area in toSegments");
            }
        }

        static std::vector<ContextItem> expand(
            const RefBuilder& refs, const std::vector<ContextItem>& context)
        {
            std::vector<ContextItem> newContext;
            for (const auto& c : context)
            {
                std::vector<ContextItem> add = c.expand(refs);
                if (!add.empty())
                {
                    newContext.insert(newContext.end(), add.begin(), add.end());
                }
            }
            return newContext;
        }

        std::vector<ContextItem> expand(const RefBuilder& refs) const
        {
            if (expanding)
            {
                switch (type)
                {
                    case Type::String:
                        throw StringException("Unreachable area in ContextItem::expand");
                    case Type::Multiname:
                    {
                        throw StringException("Not sure this can be handled properly: expand got "
                                              "an expanding multiname");
                    }
                    case Type::Group:
                        return {ContextItem(std::get<GroupData>(data).groupFallback)};
                }
            }

            if (expanding)
            {
                throw StringException("Somehow here while expanding?");
            }

            struct ExpandingScopeGuard
            {
                bool& expanding;

                ExpandingScopeGuard(bool& expanding) : expanding(expanding) { expanding = true; }

                ~ExpandingScopeGuard() { expanding = false; }
            };

            ExpandingScopeGuard esg = {expanding};

            switch (type)
            {
                case Type::Multiname:
                {
                    auto& multiname = std::get<ASASM::Multiname>(data);
                    switch (multiname.kind)
                    {
                        case ABCType::QName:
                        {
                            const auto& ns = multiname.qname().ns;
                            if (ns.kind == ABCType::PrivateNamespace)
                            {
                                auto ctx =
                                    refs.namespaces[(uint8_t)ns.kind].getContext(refs, ns.id);
                                if (multiname.qname().name)
                                {
                                    ctx.emplace_back(*multiname.qname().name);
                                }
                                return ctx;
                            }
                        }
                        break;
                        case ABCType::Multiname:
                            return multiname.multiname().name
                                     ? std::vector<ContextItem>{ContextItem(
                                           *multiname.multiname().name)}
                                     : std::vector<ContextItem>{};
                        default:
                            throw StringException("Incorrect multiname kind in expand");
                    }
                }

                case Type::String:
                    break;
                case Type::Group:
                    return reduceGroup(refs);
            }
            // This copy needs to be not expanding
            expanding = false;
            return {*this};
        }

        ContextItem(const ASASM::Multiname& m) : type(Type::Multiname), data(m) {}

        ContextItem(const std::string& s, bool filenameSuffix = false)
            : type(Type::String), data(StrData{s, filenameSuffix})
        {
        }

        ContextItem(const char* const s, bool filenameSuffix = false)
            : type(Type::String), data(StrData{s, filenameSuffix})
        {
        }

        ContextItem(const std::vector<ContextItem>& group, const std::string& groupFallback)
            : type(Type::Group), data(GroupData{group, groupFallback})
        {
        }

        auto operator<=>(const ContextItem&) const noexcept = default;
        bool operator==(const ContextItem&) const noexcept  = default;

        static bool similar(const ContextItem& l, const ContextItem& r)
        {
            if (l.type != r.type)
            {
                return false;
            }
            switch (l.type)
            {
                case Type::String:
                    return std::get<StrData>(l.data).str == std::get<StrData>(r.data).str;
                case Type::Multiname:
                    if (std::get<ASASM::Multiname>(l.data).kind != ABCType::QName ||
                        std::get<ASASM::Multiname>(r.data).kind != ABCType::QName)
                    {
                        throw StringException("Incorrect multiname context item types in similar");
                    }
                    if (std::get<ASASM::Multiname>(l.data).qname().name !=
                        std::get<ASASM::Multiname>(r.data).qname().name)
                    {
                        return false;
                    }
                    return nsSimilar(std::get<ASASM::Multiname>(l.data).qname().ns,
                        std::get<ASASM::Multiname>(r.data).qname().ns);
                case Type::Group:
                    return std::get<GroupData>(l.data).group == std::get<GroupData>(r.data).group;
            }
            throw StringException("Unreachable area in similar");
        }

        template <bool truncate>
        static std::vector<ContextItem> combine(ContextItem& c1, ContextItem& c2)
        {
            if (similar(c1, c2))
            {
                return {c1};
            }

            if (c1.type != Type::Multiname || c2.type != Type::Multiname)
            {
                return {};
            }
            if (std::get<ASASM::Multiname>(c1.data).kind != ABCType::QName ||
                std::get<ASASM::Multiname>(c2.data).kind != ABCType::QName)
            {
                return {};
            }

            auto& name1 = std::get<ASASM::Multiname>(c1.data).qname().name;
            auto& name2 = std::get<ASASM::Multiname>(c1.data).qname().name;
            auto& ns1   = std::get<ASASM::Multiname>(c1.data).qname().ns;
            auto& ns2   = std::get<ASASM::Multiname>(c1.data).qname().ns;

            if constexpr (truncate)
            {
                if (nsSimilar(ns1, ns2) && ns1.name)
                {
                    ASASM::Multiname m;
                    m.kind = ABCType::QName;
                    m.qname(ASASM::Multiname::_QName{ns1});
                    return {ContextItem{m}};
                }

                if (name1 && !name2)
                {
                    std::swap(c1, c2);
                    std::swap(ns1, ns2);
                    std::swap(name1, name2);
                }
            }

            if (!name1 && name2 && nsSimilar(ns1, ns2))
            {
                if constexpr (truncate)
                {
                    ASASM::Multiname m;
                    m.kind = ABCType::QName;
                    m.qname(ASASM::Multiname::_QName{ns1});
                    return {ContextItem(m)};
                }
                else
                {
                    return {c2};
                }
            }

            if (ns1.name && ns2.name)
            {
                if (nsSimilar(ns1, ns2))
                {
                    if (name1 == name2)
                    {
                        throw StringException("Already handled truncate case");
                    }
                    if constexpr (truncate)
                    {
                        throw StringException("Also already handled truncate case");
                    }
                    else
                    {
                        if (name2)
                        {
                            return {c1, ContextItem(*name2)};
                        }
                        else
                        {
                            return {c1};
                        }
                    }
                }

                if constexpr (truncate)
                {
                    if (ns1.name->size() > ns2.name->size())
                    {
                        std::swap(c1, c2);
                        std::swap(ns1, ns2);
                        std::swap(name1, name2);
                    }
                }

                std::string fullName1 = *ns1.name + (name1 ? ":" + *name1 : "");
                std::string fullName2 = *ns2.name + (name2 ? ":" + *name2 : "");
                if (fullName2.starts_with(fullName1 + ":"))
                {
                    return {truncate ? c1 : c2};
                }
            }

            return {};
        }

        static std::vector<ContextItem> commonRoot(ContextItem& c1, ContextItem& c2)
        {
            return combine<true>(c1, c2);
        }

        static std::vector<ContextItem> deduplicate(ContextItem& c1, ContextItem& c2)
        {
            return combine<false>(c1, c2);
        }

        static std::vector<ContextItem> contextRoot(
            std::vector<ContextItem>& c1, std::vector<ContextItem>& c2)
        {
            constexpr auto uninteresting = [](const std::vector<ContextItem>& c)
            {
                for (const auto& cc : c)
                {
                    if (cc.type == Type::Group)
                    {
                        throw StringException("Group in context root");
                    }
                }

                if (c.size() != 1)
                {
                    return false;
                }

                if (c[0].type != Type::String)
                {
                    return false;
                }

                return (std::get<StrData>(c[0].data).str.starts_with("script_") &&
                           std::get<StrData>(c[0].data).str.ends_with("_sinit")) ||
                       (std::get<StrData>(c[0].data).str.starts_with("orphan_method_"));
            };

            if (uninteresting(c1))
            {
                return c2;
            }
            if (uninteresting(c2))
            {
                return c1;
            }

            std::vector<ContextItem> c;
            while (c.size() < c1.size() && c.size() < c2.size())
            {
                auto root = commonRoot(c1[c.size()], c2[c.size()]);
                if (root.size() > 1)
                {
                    throw StringException("Common root too big");
                }
                if (!root.empty())
                {
                    c.emplace_back(std::move(root[0]));
                }
                else
                {
                    break;
                }
            }
            return c;
        }
    };

    std::vector<ContextItem> context;

    template <typename... Ts>
    void pushContext(Ts... args)
    {
        context.emplace_back(args...);
    }

    void popContext() { context.pop_back(); }

    enum class ContextPriority : uint8_t
    {
        declaration,
        usage,
        orphan,
        max
    };

    template <typename t, bool ALLOW_DUPLICATES>
    struct ContextSet
    {
        using T = std::remove_cvref_t<t>;
        mutable std::unordered_map<T, std::vector<ContextItem>> contexts;
        std::unordered_map<T, std::array<std::vector<std::vector<ContextItem>>, 3>> contextSets;
#ifndef NDEBUG
        mutable bool contextsSealed = false;
        mutable bool coagulated     = false;
#endif

        std::unordered_map<T, std::string> names, filenames;

        bool add(T obj, const std::vector<ContextItem>& context, ContextPriority priority)
        {
            assert(!coagulated);
            assert(!contextsSealed);

            if (auto found = contextSets.find(obj); found != contextSets.end())
            {
                auto& pet = found->second[(uint8_t)priority];
                if (pet.size() == 0 || pet[pet.size() - 1] != context)
                {
                    pet.emplace_back(context);
                }
                return false;
            }
            else
            {
                contextSets[obj][(uint8_t)priority].emplace_back(context);
                return true;
            }
        }

        bool addIfNew(T obj, const std::vector<ContextItem>& context, ContextPriority priority)
        {
            if (isAdded(obj))
            {
                return false;
            }
            else
            {
                return add(obj, context, priority);
            }
        }

        bool isAdded(T obj) { return contextSets.contains(obj); }

        void coagulate(const RefBuilder& refs)
        {
            assert(!coagulated);

            std::unordered_map<std::string, int> collisionCounter;
            std::unordered_map<std::string, T> first;

            for (const auto& obj : contextSets)
            {
                if (!contexts.contains(obj.first))
                {
                    getContext(refs, obj.first);
                }
            }

            for (const auto& obj : contexts)
            {
                std::string bname     = refs.contextToString(obj.second, false);
                std::string bfilename = refs.contextToString(obj.second, true);
                int counter = collisionCounter.contains(bname) ? collisionCounter.at(bname) : 0;
                if (counter == 1)
                {
                    names.at(first[bname])     += "#0";
                    filenames.at(first[bname]) += "#0";
                }

                std::string suffix;
                if (counter == 0)
                {
                    first[bname] = obj.first;
                }
                else
                {
                    suffix = "#" + std::to_string(counter);
                }
                names.insert({obj.first, bname + suffix});
                filenames.insert({obj.first, bfilename + suffix});
                collisionCounter.insert_or_assign(bname, counter + 1);
            }

#ifndef NDEBUG
            coagulated = true;
#endif
        }

        std::vector<ContextItem> getContext(const RefBuilder& refs, T obj) const
        {
#ifndef NDEBUG
            contextsSealed = true;
#endif

            if (contexts.contains(obj))
            {
                return contexts.at(obj);
            }

            std::vector<std::vector<ContextItem>> set;

            for (const auto& prioritySet : contextSets.at(obj))
            {
                if (!prioritySet.empty())
                {
                    set = prioritySet;
                    break;
                }
            }

            if constexpr (ALLOW_DUPLICATES)
            {
                std::vector<ContextItem> ctx = ContextItem::expand(refs, set[0]);
                for (size_t i = 1; i < set.size(); i++)
                {
                    auto expanded = ContextItem::expand(refs, set[i]);
                    ctx           = ContextItem::contextRoot(ctx, expanded);
                }
                contexts[obj] = ctx;
            }
            else
            {
                if (set.size() > 1)
                {
                    contexts[obj] = {ContextItem("multireferenced")};
                }
                else
                {
                    contexts[obj] = ContextItem::expand(refs, set[0]);
                }
            }

            return contexts.at(obj);
        }

        std::string getName(T obj)
        {
            assert(coagulated);

            return names.at(obj);
        }

        std::unordered_map<std::string, std::string> filenameMappings;

        std::string getFilename(T obj, std::string_view suffix)
        {
            assert(coagulated);

            assert(filenames.contains(obj));
            std::string filename = filenames.at(obj);

            std::vector<std::string> dirSegments = StringUtils::split(filename, "/");
            for (size_t l = 0; l < dirSegments.size(); l++)
            {
            again:
                std::string subpath = StringUtils::join(dirSegments.data(), l, "/");
                std::string subpathl(subpath.size(), '\0');
                std::transform(subpath.begin(), subpath.end(), subpathl.begin(),
                    [](auto v) { return (char)tolower(v); });

                if (filenameMappings.contains(subpathl) && filenameMappings[subpathl] != subpath)
                {
                    dirSegments[l] += "_";
                    goto again;
                }
                filenameMappings[subpathl] = subpath;
            }
            filename = StringUtils::join(dirSegments, "/");

            std::string ret = filename + ".";
            ret             += suffix;
            ret             += ".asasm";
            return ret;
        }
    };

    std::array<ContextSet<int, true>, ABCTypeMap.GetEntries().size()> namespaces;
    ContextSet<const void*, false> objects, scripts;

    RefBuilder(const ASASM::ASProgram& as) : ASTraitsVisitor(as) {}

    std::unordered_map<const void*, bool> orphans;

    void addOrphan(const void* obj) { orphans[obj] = true; }

    bool isOrphan(const void* obj) { return orphans.contains(obj); }

    void run() override
    {
        for (size_t i = 0; i < as.orphanClasses.size(); i++)
        {
            addOrphan(as.orphanClasses[i].get());
        }
        for (size_t i = 0; i < as.orphanMethods.size(); i++)
        {
            addOrphan(as.orphanMethods[i].get());
        }

        ASTraitsVisitor::run();

        for (size_t i = 0; i < as.scripts.size(); i++)
        {
            std::vector<ContextItem> classContexts;

            for (const auto& trait : as.scripts[i]->traits)
            {
                if (trait.name.kind == ABCType::QName &&
                    trait.name.qname().ns.kind != ABCType::PrivateNamespace)
                {
                    classContexts.emplace_back(trait.name);
                }
            }

            if (classContexts.empty())
            {
                for (const auto& trait : as.scripts[i]->traits)
                {
                    classContexts.emplace_back(trait.name);
                }
            }

            context = {
                {classContexts, "script_" + std::to_string(i)}
            };
            scripts.add(&as.scripts[i], context, ContextPriority::declaration);
            pushContext("init", true);
            addMethod(as.scripts[i]->sinit, ContextPriority::declaration);
            context.clear();
        }

        for (size_t i = 0; i < as.orphanClasses.size(); i++)
        {
            if (!objects.isAdded(as.orphanClasses[i].get()))
            {
                pushContext("orphan_class_" + std::to_string(i));
                addClass(as.orphanClasses[i], ContextPriority::orphan);
                popContext();
            }
        }
        for (size_t i = 0; i < as.orphanMethods.size(); i++)
        {
            if (!objects.isAdded(as.orphanMethods[i].get()))
            {
                pushContext("orphan_method_" + std::to_string(i));
                addMethod(as.orphanMethods[i], ContextPriority::orphan);
                popContext();
            }
        }

        scripts.coagulate(*this);

        for (const auto& script : as.scripts)
        {
            for (const auto& trait : script->traits)
            {
                if (trait.name.kind == ABCType::QName)
                {
                    namespaces[(uint8_t)trait.name.qname().ns.kind].addIfNew(
                        trait.name.qname().ns.id, scripts.getContext(*this, (const void*)&script),
                        ContextPriority::declaration);
                }
            }
        }

        for (int i = 0; i < (int)possibleOrphanPrivateNamespaces.size(); i++)
        {
            if (!namespaces[(uint8_t)ABCType::PrivateNamespace].isAdded(i))
            {
                pushContext("orphan_namespace_" + std::to_string(i));
                namespaces[(uint8_t)ABCType::PrivateNamespace].add(
                    i, context, ContextPriority::orphan);
                popContext();
            }
        }

#ifndef NDEBUG
        homonymsBuilt = true;
#endif

        for (auto& ns : namespaces)
        {
            ns.coagulate(*this);
        }
        objects.coagulate(*this);
    }

    void visitTrait(const ASASM::Trait& trait) override
    {
        pushContext(trait.name);
        visitMultiname(trait.name, ContextPriority::declaration);
        switch (trait.kind)
        {
            case TraitKind::Slot:
            case TraitKind::Const:
                visitMultiname(trait.vSlot().typeName, ContextPriority::usage);

                ASTraitsVisitor::visitTrait(trait);
                break;
            case TraitKind::Class:
                addClass(trait.vClass().vclass, ContextPriority::declaration);

                pushContext("class", true);
                visitTraits(trait.vClass().vclass->traits);
                popContext();

                pushContext("instance", true);
                visitTraits(trait.vClass().vclass->instance.traits);
                popContext();
                break;
            case TraitKind::Function:
                addMethod(trait.vFunction().vfunction, ContextPriority::declaration);

                ASTraitsVisitor::visitTrait(trait);
                break;
            case TraitKind::Method:
                addMethod(trait.vMethod().vmethod, ContextPriority::declaration);

                ASTraitsVisitor::visitTrait(trait);
                break;
            case TraitKind::Getter:
                pushContext("getter");
                addMethod(trait.vMethod().vmethod, ContextPriority::declaration);
                popContext();

                ASTraitsVisitor::visitTrait(trait);
                break;
            case TraitKind::Setter:
                pushContext("setter");
                addMethod(trait.vMethod().vmethod, ContextPriority::declaration);
                popContext();

                ASTraitsVisitor::visitTrait(trait);
                break;
            default:
                ASTraitsVisitor::visitTrait(trait);
                break;
        }
        popContext();
    }

    std::unordered_map<int, bool> possibleOrphanPrivateNamespaces;

    void visitNamespace(const ASASM::Namespace& ns, ContextPriority priority)
    {
        if (ns.kind == ABCType::Void)
        {
            return;
        }

        addHomonym(ns);

        assert(context.size() > 0);

        size_t myPos = context.size();
        for (size_t i = 0; i < context.size(); i++)
        {
            if (context[i].type == ContextItem::Type::Multiname &&
                std::get<ASASM::Multiname>(context[i].data).kind == ABCType::QName &&
                std::get<ASASM::Multiname>(context[i].data).qname().ns == ns)
            {
                myPos = i;
                break;
            }
        }
        if (ns.kind == ABCType::PrivateNamespace && myPos == 0)
        {
            possibleOrphanPrivateNamespaces[ns.id] = true;
            return;
        }

        std::vector<ContextItem> myContext{context.begin(), context.begin() + myPos};
        namespaces[(uint8_t)ns.kind].add(ns.id, myContext, priority);
    }

    void visitNamespaceSet(const std::vector<ASASM::Namespace>& nsSet, ContextPriority priority)
    {
        for (const auto& ns : nsSet)
        {
            visitNamespace(ns, priority);
        }
    }

    void visitMultiname(const ASASM::Multiname& m, ContextPriority priority)
    {
        if (m.kind == ABCType::Void)
        {
            return;
        }
        switch (m.kind)
        {
            case ABCType::QName:
            case ABCType::QNameA:
                visitNamespace(m.qname().ns, priority);
                break;
            case ABCType::Multiname:
            case ABCType::MultinameA:
                visitNamespaceSet(m.multiname().nsSet, priority);
                break;
            case ABCType::MultinameL:
            case ABCType::MultinameLA:
                visitNamespaceSet(m.multinamel().nsSet, priority);
                break;
            case ABCType::TypeName:
                visitMultiname(m.Typename().name(), priority);
                for (const auto& param : m.Typename().params())
                {
                    visitMultiname(param, priority);
                }
                break;
            default:
                break;
        }
    }

    void visitMethodBody(const ASASM::MethodBody& b)
    {
        for (const auto& instruction : b.instructions)
        {
            for (size_t i = 0; i < OPCode_Info[(uint8_t)instruction.opcode].second.size(); i++)
            {
                switch (OPCode_Info[(uint8_t)instruction.opcode].second[i])
                {
                    case OPCodeArgumentType::Namespace:
                        visitNamespace(
                            instruction.arguments[i].namespacev(), ContextPriority::usage);
                        break;
                    case OPCodeArgumentType::Multiname:
                        visitMultiname(
                            instruction.arguments[i].multinamev(), ContextPriority::usage);
                        break;
                    case OPCodeArgumentType::Class:
                        pushContext("inline_class");
                        if (isOrphan(instruction.arguments[i].classv().get()))
                        {
                            addClass(instruction.arguments[i].classv(), ContextPriority::usage);
                        }
                        popContext();
                        break;
                    case OPCodeArgumentType::Method:
                        pushContext("inline_method");
                        if (isOrphan(instruction.arguments[i].methodv().get()))
                        {
                            addMethod(instruction.arguments[i].methodv(), ContextPriority::usage);
                        }
                        popContext();
                        break;
                    default:
                        break;
                }
            }
        }
    }

    std::string contextToString(std::vector<ContextItem> ctx, bool filename) const
    {
        ctx = ContextItem::expand(*this, ctx);
        if (ctx.empty())
        {
            return "";
        }

        for (size_t i = ctx.size() - 1; i > 0; i--)
        {
            auto root = ContextItem::deduplicate(ctx[i - 1], ctx[i]);
            if (!root.empty())
            {
                std::vector<ContextItem> newContext{ctx.begin(), ctx.begin() + i - 1};
                newContext.insert(newContext.end(), root.begin(), root.end());
                newContext.insert(newContext.end(), ctx.begin() + i + 1, ctx.end());
                ctx = std::move(newContext);
            }
        }

        std::vector<ContextItem::Segment> segments;
        for (const auto& ci : ctx)
        {
            auto newSegments = ci.toSegments(*this, filename);
            segments.insert(segments.end(), newSegments.begin(), newSegments.end());
        }

        auto escape = [&](std::string_view s)
        {
            if (!filename)
            {
                return std::string(s);
            }

            std::string ret;
            for (char c : s)
            {
                if (c == '.' || c == ':')
                {
                    ret += '/';
                }
                else if (c == '\\' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' ||
                         c == '|' || c < 0x20 || c >= 0x7F || c == ' ' || c == '%')
                {
                    char output[5] = {'\0'};
                    sprintf(output, "%%%02X", c);
                    ret += output;
                }
                else
                {
                    ret += c;
                }
            }

            auto pathSegments = StringUtils::split(ret, "/");
            if (pathSegments.empty())
            {
                pathSegments = {""};
            }
            for (auto& segment : pathSegments)
            {
                if (segment == "")
                {
                    segment = "%";
                }

                static constexpr std::array<std::string_view, 22> reservedNames = {"CON", "PRN",
                    "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                    "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"};

                std::string segmentU(segment.size(), '\0');
                std::transform(segment.begin(), segment.end(), segmentU.begin(),
                    [](auto v) { return (char)toupper(v); });
                for (const auto& reservedName : reservedNames)
                {
                    if (segmentU.starts_with(reservedName) &&
                        (segmentU.size() == reservedName.size() ||
                            segmentU[reservedName.size()] == '.'))
                    {
                        segment = "%" + segment;
                    }
                }

                // TODO: see whether this is really necessary, and, if so, add it
                // if (segment.size() > 240)
                // {
                //     segment = segment.substr(0, 200) + '-' + toHexString(md5Of(segment));
                // }
            }

            return StringUtils::join(pathSegments, "/");
        };

        std::vector<std::string> strings(segments.size());
        for (size_t i = 0; i < segments.size(); i++)
        {
            if (i > 0)
            {
                strings[i] = segments[i].delim + escape(segments[i].str);
            }
            else
            {
                strings[i] = escape(segments[i].str);
            }
        }

        return StringUtils::join(strings, "");
    }

    template <typename T>
    bool addObject(T obj, ContextPriority priority)
    {
        return objects.add(obj.get(), context, priority);
    }

    void addClass(const std::shared_ptr<ASASM::Class> vclass, ContextPriority priority)
    {
        addObject(vclass, priority);

        pushContext("class", true);
        pushContext("init", true);
        addMethod(vclass->cinit, ContextPriority::declaration);
        popContext();
        popContext();

        pushContext("instance", true);
        pushContext("init", true);
        addMethod(vclass->instance.iinit, ContextPriority::declaration);
        popContext();

        visitMultiname(vclass->instance.name, ContextPriority::declaration);
        visitMultiname(vclass->instance.superName, ContextPriority::usage);
        visitNamespace(vclass->instance.protectedNs, ContextPriority::declaration);
        for (const auto& iface : vclass->instance.interfaces)
        {
            visitMultiname(iface, ContextPriority::usage);
        }
        popContext();
    }

    void addMethod(const std::shared_ptr<ASASM::Method> method, ContextPriority priority)
    {
        if (addObject(method, priority))
        {
            for (const auto& paramType : method->paramTypes)
            {
                visitMultiname(paramType, ContextPriority::usage);
            }
            visitMultiname(method->returnType, ContextPriority::usage);
            if (method->vbody)
            {
                visitMethodBody(*method->vbody);
            }
        }
    }
};
