#pragma once

#include "ASASM/ASProgram.hpp"
#include "ASASM/Class.hpp"
#include "ASASM/Exception.hpp"
#include "ASASM/Instance.hpp"
#include "ASASM/Instruction.hpp"
#include "ASASM/Metadata.hpp"
#include "ASASM/Method.hpp"
#include "ASASM/MethodBody.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Namespace.hpp"
#include "ASASM/Script.hpp"
#include "ASASM/Trait.hpp"
#include "ASASM/Value.hpp"
#include "enums/ABCType.hpp"
#include "enums/InstanceFlags.hpp"
#include "enums/MethodFlags.hpp"
#include "enums/TraitAttribute.hpp"
#include "utils/StringException.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Assembler
{
private:
    const std::unordered_map<std::string, std::string>& strings;
    bool includeDebugInstructions;

    static constexpr char addUniqueMethod[] = "method";
    static constexpr char addUniqueClass[]  = "class";
    static constexpr char addUniqueLabel[]  = "label";

    struct SourceFile : public std::enable_shared_from_this<SourceFile>
    {
        std::string name;
        std::vector<std::string> arguments;
        std::string data;

        bool isVirtual() { return data != ""; }

        std::shared_ptr<SourceFile> parent;

        size_t filePosition;

        SourceFile(std::string_view name, std::string_view data,
            const std::vector<std::string>& arguments = {})
            : name(name), arguments(arguments), data(data), filePosition(0)
        {
            if (data.empty())
            {
                throw StringException("Empty source file");
            }
        }

        struct Position
        {
            std::shared_ptr<SourceFile> file;
            size_t offset;

            const std::shared_ptr<SourceFile>& load() const
            {
                file->filePosition = offset;
                return file;
            }
        };

        Position position() { return Position(shared_from_this(), filePosition); }

        std::string positionStr()
        {
            size_t lineStart = 0;
            size_t line      = 1;
            for (size_t i = 0; i < data.size(); i++)
            {
                if (i == filePosition)
                {
                    return name + "(" + std::to_string(line) + "," +
                           std::to_string(i - lineStart + 1) + ")";
                }
                if (data[i] == '\n')
                {
                    line++;
                    lineStart = i;
                }
            }

            return name + "(???)";
        }

        char front() { return data[filePosition]; }

        void popFront() { filePosition++; }
    };

    using Position = SourceFile::Position;

    std::shared_ptr<SourceFile> currentFile;

    void skipWhitespace()
    {
        while (true)
        {
            char c;
            while ((c = peekChar()) == 0)
            {
                popFile();
            }
            if (c == ' ' || c == '\r' || c == '\n' || c == '\t')
            {
                skipChar();
            }
            else if (c == '#')
            {
                handlePreprocessor();
            }
            else if (c == '$')
            {
                handleVar();
            }
            else if (c == ';')
            {
                do
                {
                    skipChar();
                }
                while (peekChar() != '\n');
            }
            else
            {
                return;
            }
        }
    }

    std::unordered_map<std::string, std::string> vars;
    std::vector<std::string> namespaceLabels;
    uint32_t sourceVersion = 1;

    void handlePreprocessor()
    {
        skipChar(); // #
        std::string word = readWord();

        if (word == "mixin")
        {
            pushFile(std::make_shared<SourceFile>("#mixin", readString()));
        }
        else if (word == "call")
        {
            auto rsLambda = [this] { return readString(); };
            pushFile(std::make_shared<SourceFile>(
                "#call", readString(), readList<'(', ')', false>([this] { return readString(); })));
        }
        else if (word == "include")
        {
            std::string s = readString();
            pushFile(std::make_shared<SourceFile>(s, strings.at(s)));
        }
        else if (word == "get")
        {
            std::string s = readString();
            pushFile(std::make_shared<SourceFile>(s, toStringLiteral(strings.at(s))));
        }
        else if (word == "set")
        {
            vars[readWord()] = readString();
        }
        else if (word == "unset")
        {
            vars.erase(readWord());
        }
        else if (word == "privatens")
        {
            if (sourceVersion >= 3)
            {
                throw StringException("#privatens is deprecated");
            }
            readUInt();
            readString();
        }
        else if (word == "version")
        {
            sourceVersion = (uint32_t)readUInt();
            if (sourceVersion < 1 || sourceVersion > 4)
            {
                throw StringException("Invalid/unknown #version");
            }
        }
        else
        {
            backpedal(word.size());
            throw StringException("Unknown preprocessor declaration: " + word);
        }
    }

    void handleVar()
    {
        skipChar(); // $
        skipWhitespace();

        std::string name;
        bool asStringLiteral = false;
        if (peekChar() == '"')
        {
            name            = readString();
            asStringLiteral = true;
        }
        else
        {
            name = readWord();
        }

        if (name.size() == 0)
        {
            throw StringException("Empty variable name");
        }
        if (name[0] >= '1' && name[0] <= '9')
        {
            for (std::shared_ptr<SourceFile> f = currentFile; f != nullptr; f = f->parent)
            {
                if (!f->arguments.empty())
                {
                    size_t index = atoi(name.c_str()) - 1;
                    if (index >= f->arguments.size())
                    {
                        throw StringException("Argument index out of bounds");
                    }
                    pushFile(std::make_shared<SourceFile>(
                        '$' + name, asStringLiteral ? toStringLiteral(f->arguments[index])
                                                    : f->arguments[index]));
                    return;
                }
            }
            throw StringException("No arguments in context");
        }
        else
        {
            if (!vars.contains(name))
            {
                throw StringException("Variable " + name + " is not defined");
            }
            pushFile(std::make_shared<SourceFile>(
                '$' + name, asStringLiteral ? toStringLiteral(vars.at(name)) : vars.at(name)));
        }
    }

    static bool isWordChar(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
               c == '_' || c == '-' || c == '+' || c == '.';
    }

    std::string readWord()
    {
        skipWhitespace();
        if (!isWordChar(currentFile->front()))
        {
            // throw StringException("Word character expected");
            return {};
        }
        std::string ret;
        while (true)
        {
            char c = currentFile->front();
            if (!isWordChar(c))
            {
                break;
            }
            ret += c;
            currentFile->popFront();
        }
        return ret;
    }

    uint8_t fromHex(char x)
    {
        switch (x)
        {
            case '0':
                return 0x0;
            case '1':
                return 0x1;
            case '2':
                return 0x2;
            case '3':
                return 0x3;
            case '4':
                return 0x4;
            case '5':
                return 0x5;
            case '6':
                return 0x6;
            case '7':
                return 0x7;
            case '8':
                return 0x8;
            case '9':
                return 0x9;
            case 'a':
            case 'A':
                return 0xa;
            case 'b':
            case 'B':
                return 0xb;
            case 'c':
            case 'C':
                return 0xc;
            case 'd':
            case 'D':
                return 0xd;
            case 'e':
            case 'E':
                return 0xe;
            case 'f':
            case 'F':
                return 0xf;
            default:
                throw StringException(std::string("Malformed hex digit ") + x);
        }
    }

    void pushFile(std::shared_ptr<SourceFile> f)
    {
        f->parent   = currentFile;
        currentFile = f;
    }

    void setFile(std::shared_ptr<SourceFile> f) { currentFile = f; }

    void popFile()
    {
        if (!currentFile || !currentFile->parent)
        {
            throw StringException("Unexpected end of file");
        }
        currentFile = currentFile->parent;
    }

    void expectWord(std::string_view expected)
    {
        std::string word = readWord();
        if (word != expected)
        {
            backpedal(word.size());
            throw StringException("Expected " + std::string(expected));
        }
    }

    char peekChar() { return currentFile->front(); }

    void skipChar() { currentFile->popFront(); }

    void backpedal(size_t amount = 1) { currentFile->filePosition -= amount; }

    char readChar()
    {
        char ret = peekChar();
        if (ret != '\0')
        {
            skipChar();
        }
        return ret;
    }

    char readSymbol()
    {
        skipWhitespace();
        return readChar();
    }

    void expectSymbol(char c)
    {
        if (readSymbol() != c)
        {
            backpedal();
            throw StringException(std::string("Expected ") + c);
        }
    }

    // *****************************

    template <typename T>
    void mustBeNull(const T& obj)
    {
        using comp                 = std::remove_cvref_t<T>;
        static constexpr auto fail = [] { throw StringException("Repeated field declaration"); };
        if constexpr (std::is_same_v<ASASM::Multiname, comp> ||
                      std::is_same_v<ASASM::Namespace, comp>)
        {
            if (obj.kind != ABCType::Void)
            {
                fail();
            }
        }
        else if constexpr (std::is_same_v<std::shared_ptr<ASASM::Class>, comp> ||
                           std::is_same_v<std::shared_ptr<ASASM::Method>, comp>)
        {
            if (obj != nullptr)
            {
                fail();
            }
        }
        else if constexpr (std::is_same_v<std::string, comp>)
        {
            if (!obj.empty())
            {
                fail();
            }
        }
        else if constexpr (std::is_same_v<ASASM::Instance, comp>)
        {
            mustBeNull(obj.iinit);
        }
        else
        {
            static_assert(std::is_same_v<void, comp>);
        }
    }

    template <typename T>
    void mustBeSet(const char* name, const T& obj)
    {
        using comp = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<std::shared_ptr<ASASM::Method>, comp>)
        {
            if (obj == nullptr)
            {
                throw StringException(std::string(name) + " not set");
            }
        }
        else if constexpr (std::is_same_v<ASASM::Instance, comp>)
        {
            mustBeSet(name, obj.iinit);
        }
        else
        {
            static_assert(std::is_same_v<void, comp> && std::is_same_v<uint8_t, comp>);
        }
    }

    static ABCType toABCType(std::string_view name)
    {
        auto found = ABCTypeMap.Find(name.data());
        if (!found)
        {
            throw StringException("Unknown ABCType " + std::string(name));
        }
        return *found;
    }

    template <const char* name, typename K, typename V>
    static void addUnique(std::unordered_map<K, V>& aa, K k, V v)
    {
        if (aa.contains(k))
        {
            throw StringException("Duplicate " + std::string(name));
        }
        aa.emplace(k, v);
    }

    static std::string toStringLiteral(std::string_view str)
    {
        // if (str.empty()) return "null"; else
        {
            static constexpr std::array<char, 16> hexDigits = {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

            std::string ret = "\"";
            ret.reserve(str.size());
            for (char c : str)
            {
                if (c == '\n')
                {
                    ret += "\\n";
                }
                else if (c == '\r')
                {
                    ret += "\\r";
                }
                else if (c == '\\')
                {
                    ret += "\\\\";
                }
                else if (c == '"')
                {
                    ret += "\\\"";
                }
                else
                {
                    unsigned char uc = c;
                    if (uc < 0x20 || uc >= 0x7F)
                    {
                        ret += "\\x";
                        ret += hexDigits[uc / 0x10];
                        ret += hexDigits[uc % 0x10];
                    }
                    else
                    {
                        ret += c;
                    }
                }
            }
            ret += '"';

            return ret;
        }
    }

    ASASM::Value readValue()
    {
        ASASM::Value ret;
        ret.vkind = toABCType(readWord());
        expectSymbol('(');
        switch (ret.vkind)
        {
            case ABCType::Integer:
                ret.vint(readInt());
                break;
            case ABCType::UInteger:
                ret.vuint(readUInt());
                break;
            case ABCType::Double:
                ret.vdouble(readDouble());
                break;
            case ABCType::Utf8:
                ret.vstring(readString());
                break;
            case ABCType::Namespace:
            case ABCType::PackageNamespace:
            case ABCType::PackageInternalNs:
            case ABCType::ProtectedNamespace:
            case ABCType::ExplicitNamespace:
            case ABCType::StaticProtectedNs:
            case ABCType::PrivateNamespace:
                ret.vnamespace(readNamespace());
                break;
            case ABCType::True:
            case ABCType::False:
            case ABCType::Null:
            case ABCType::Undefined:
                break;
            default:
                throw StringException("Unknown type");
        }
        expectSymbol(')');
        return ret;
    }

    template <typename T>
    uint8_t readFlag(const T& BiMap)
    {
        auto word  = readWord();
        auto found = BiMap.Find(word.c_str());
        if (!found)
        {
            backpedal(word.size());
            throw StringException("Unknown flag " + word);
        }
        return (uint8_t)found->get();
    }

    template <char OPEN, char CLOSE, bool ALLOW_NULL, typename Reader>
    std::vector<decltype(std::declval<Reader>()())> readList(const Reader& r)
    {
        if constexpr (ALLOW_NULL)
        {
            skipWhitespace();
            if (peekChar() != OPEN)
            {
                std::string word = readWord();
                if (word != "null")
                {
                    backpedal(word.size());
                    throw StringException(std::string("Expected ") + OPEN + " or null");
                }
                return {};
            }
        }

        expectSymbol(OPEN);
        std::vector<decltype(std::declval<Reader>()())> ret;

        skipWhitespace();
        if (peekChar() == CLOSE)
        {
            skipChar(); // CLOSE
            return {};
        }
        while (true)
        {
            ret.emplace_back(r());
            char c = readSymbol();
            if (c == CLOSE)
            {
                break;
            }
            if (c != ',')
            {
                backpedal();
                throw StringException(std::string("Expected ") + CLOSE + " or ,");
            }
        }
        return ret;
    }

    int64_t readInt()
    {
        const std::string w = readWord();
        if (w == "null")
        {
            return SWFABC::ABCFile::NULL_INT;
        }
        const int64_t ret = strtoll(w.c_str(), nullptr, 0);
        if (ret < SWFABC::ABCFile::MIN_INT || ret > SWFABC::ABCFile::MAX_INT)
        {
            throw StringException("Int out of bounds");
        }
        return ret;
    }

    uint64_t readUInt()
    {
        const std::string w = readWord();
        if (w == "null")
        {
            return SWFABC::ABCFile::NULL_UINT;
        }
        const uint64_t ret = strtoull(w.c_str(), nullptr, 0);
        if (ret > SWFABC::ABCFile::MAX_UINT)
        {
            throw StringException("UInt out of bounds");
        }
        return ret;
    }

    double readDouble()
    {
        const std::string w = readWord();
        if (w == "null")
        {
            return SWFABC::ABCFile::NULL_DOUBLE;
        }
        return strtod(w.c_str(), nullptr);
    }

    std::string readString()
    {
        skipWhitespace();
        char c = readSymbol();
        if (c != '"')
        {
            std::string word = readWord();
            if (c == 'n' && word == "ull")
            {
                return {};
            }
            else
            {
                backpedal(1 + word.size());
                throw StringException("String literal expected");
            }
        }

        std::string ret;
        while (true)
        {
            switch (c = readChar())
            {
                case '"':
                    return ret;
                case '\\':
                {
                    switch (c = readChar())
                    {
                        case 'n':
                            ret += '\n';
                            break;
                        case 'r':
                            ret += '\r';
                            break;
                        case 'x':
                        {
                            char c0 = readChar();
                            char c1 = readChar();
                            ret     += (char)((fromHex(c0) << 4) | fromHex(c1));
                        }
                        break;
                        default:
                            ret += c;
                            break;
                    }
                }
                break;
                case '\0':
                    throw StringException("Unexpected null terminator");
                default:
                    ret += c;
                    break;
            }
        }
    }

    ASASM::Namespace readNamespace()
    {
        std::string word = readWord();
        if (word == "null")
        {
            return {};
        }

        ABCType kind = toABCType(word);
        expectSymbol('(');
        std::string name = readString();
        int id           = 0;
        if (peekChar() == ',')
        {
            skipChar();
            std::string s = readString();
            auto found    = std::find(namespaceLabels.begin(), namespaceLabels.end(), s);
            if (found != namespaceLabels.end())
            {
                id = std::distance(namespaceLabels.begin(), found) + 1;
            }
            else
            {
                namespaceLabels.emplace_back(std::move(s));
                id = namespaceLabels.size();
            }
        }
        expectSymbol(')');

        return ASASM::Namespace{kind, std::move(name), id};
    }

    std::vector<ASASM::Namespace> readNamespaceSet()
    {
        return readList<'[', ']', true>([this] { return readNamespace(); });
    }

    ASASM::Multiname readMultiname()
    {
        std::string word = readWord();
        if (word == "null")
        {
            return {};
        }
        ASASM::Multiname ret;
        ret.kind = toABCType(word);
        expectSymbol('(');
        switch (ret.kind)
        {
            case ABCType::QName:
            case ABCType::QNameA:
            {
                ASASM::Multiname::_QName qname;
                qname.ns = readNamespace();
                expectSymbol(',');
                qname.name = readString();
                ret.qname(qname);
            }
            break;
            case ABCType::RTQName:
            case ABCType::RTQNameA:
                ret.rtqname({readString()});
                break;
            case ABCType::RTQNameL:
            case ABCType::RTQNameLA:
                ret.rtqnamel({});
                break;
            case ABCType::Multiname:
            case ABCType::MultinameA:
            {
                ASASM::Multiname::_Multiname multiname;
                multiname.name = readString();
                expectSymbol(',');
                multiname.nsSet = readNamespaceSet();
                ret.multiname(multiname);
            }
            break;
            case ABCType::MultinameL:
            case ABCType::MultinameLA:
                ret.multinamel({readNamespaceSet()});
                break;
            case ABCType::TypeName:
            {
                ASASM::Multiname::_Typename Typename;
                Typename.name()   = readMultiname();
                Typename.params() = readList<'<', '>', false>([this] { return readMultiname(); });
                ret.Typename(Typename);
            }
            break;
            default:
                throw StringException("Unknown Multiname kind");
        }
        expectSymbol(')');
        return ret;
    }

    std::unordered_map<std::string, std::shared_ptr<ASASM::Class>> classesByID;
    std::unordered_map<std::string, std::shared_ptr<ASASM::Method>> methodsByID;

    template <typename T>
    struct Fixup
    {
        Position where;
        std::shared_ptr<T>& ptr;
        std::string name;
    };

    std::vector<Fixup<ASASM::Class>> classFixups;
    std::vector<Fixup<ASASM::Method>> methodFixups;

    ASASM::Trait readTrait()
    {
        ASASM::Trait ret;
        std::string kind = readWord();
        auto foundKind   = TraitKindMap.Find(kind.c_str());
        if (!foundKind)
        {
            backpedal(kind.size());
            throw StringException("Unknown trait kind");
        }
        ret.kind = *foundKind;
        ret.name = readMultiname();
        switch (ret.kind)
        {
            case TraitKind::Slot:
            case TraitKind::Const:
                ret.vSlot({});
                while (true)
                {
                    std::string word = readWord();
                    if (word == "flag")
                    {
                        ret.attributes |= readFlag(TraitAttributeMap);
                    }
                    else if (word == "slotid")
                    {
                        ret.vSlot().slotId = (uint32_t)readUInt();
                    }
                    else if (word == "type")
                    {
                        mustBeNull(ret.vSlot().typeName);
                        ret.vSlot().typeName = readMultiname();
                    }
                    else if (word == "value")
                    {
                        ret.vSlot().value = readValue();
                    }
                    else if (word == "metadata")
                    {
                        ret.metadata.emplace_back(readMetadata());
                    }
                    else if (word == "end")
                    {
                        return ret;
                    }
                    else
                    {
                        backpedal(word.size());
                        throw StringException("Unknown " + kind + " trait field " + word);
                    }
                }
                break;
            case TraitKind::Class:
                ret.vClass({});
                while (true)
                {
                    std::string word = readWord();
                    if (word == "flag")
                    {
                        ret.attributes |= readFlag(TraitAttributeMap);
                    }
                    else if (word == "slotid")
                    {
                        ret.vClass().slotId = (uint32_t)readUInt();
                    }
                    else if (word == "class")
                    {
                        mustBeNull(ret.vClass().vclass);
                        ret.vClass().vclass = readClass();
                    }
                    else if (word == "metadata")
                    {
                        ret.metadata.emplace_back(readMetadata());
                    }
                    else if (word == "end")
                    {
                        return ret;
                    }
                    else
                    {
                        backpedal(word.size());
                        throw StringException("Unknown " + kind + " trait field " + word);
                    }
                }
                break;
            case TraitKind::Function:
                ret.vFunction({});
                while (true)
                {
                    std::string word = readWord();
                    if (word == "flag")
                    {
                        ret.attributes |= readFlag(TraitAttributeMap);
                    }
                    else if (word == "slotid")
                    {
                        ret.vFunction().slotId = (uint32_t)readUInt();
                    }
                    else if (word == "method")
                    {
                        mustBeNull(ret.vFunction().vfunction);
                        ret.vFunction().vfunction = readMethod();
                    }
                    else if (word == "metadata")
                    {
                        ret.metadata.emplace_back(readMetadata());
                    }
                    else if (word == "end")
                    {
                        return ret;
                    }
                    else
                    {
                        backpedal(word.size());
                        throw StringException("Unknown " + kind + " trait field " + word);
                    }
                }
                break;
            case TraitKind::Method:
            case TraitKind::Getter:
            case TraitKind::Setter:
                ret.vMethod({});
                while (true)
                {
                    std::string word = readWord();
                    if (word == "flag")
                    {
                        ret.attributes |= readFlag(TraitAttributeMap);
                    }
                    else if (word == "dispid")
                    {
                        ret.vMethod().dispId = (uint32_t)readUInt();
                    }
                    else if (word == "method")
                    {
                        mustBeNull(ret.vMethod().vmethod);
                        ret.vMethod().vmethod = readMethod();
                    }
                    else if (word == "metadata")
                    {
                        ret.metadata.emplace_back(readMetadata());
                    }
                    else if (word == "end")
                    {
                        return ret;
                    }
                    else
                    {
                        backpedal(word.size());
                        throw StringException("Unknown " + kind + " trait field " + word);
                    }
                }
                break;
            default:
                throw StringException("Unknown trait kind");
        }
    }

    ASASM::Metadata readMetadata()
    {
        ASASM::Metadata ret;
        ret.name = readString();
        std::vector<std::string> items;
        while (true)
        {
            std::string word = readWord();
            if (word == "item")
            {
                items.emplace_back(readString());
                items.emplace_back(readString());
            }
            else if (word == "end")
            {
                if (sourceVersion < 2)
                {
                    for (size_t i = 0; i < items.size() / 2; i++)
                    {
                        ret.data.emplace_back(
                            std::move(items[i]), std::move(items[i + items.size() / 2]));
                    }
                }
                else
                {
                    for (size_t i = 0; i < items.size(); i += 2)
                    {
                        ret.data.emplace_back(std::move(items[i]), std::move(items[i + 1]));
                    }
                }
                return ret;
            }
            else
            {
                throw StringException("Expected 'item' or 'end'");
            }
        }
    }

    std::shared_ptr<ASASM::Method> readMethod()
    {
        auto ret = std::make_shared<ASASM::Method>();

        while (true)
        {
            std::string word = readWord();
            if (word == "name")
            {
                mustBeNull(ret->name);
                ret->name = readString();
            }
            else if (word == "refid")
            {
                addUnique<addUniqueMethod>(methodsByID, readString(), ret);
            }
            else if (word == "param")
            {
                ret->paramTypes.emplace_back(readMultiname());
            }
            else if (word == "returns")
            {
                mustBeNull(ret->returnType);
                ret->returnType = readMultiname();
            }
            else if (word == "flag")
            {
                ret->flags |= readFlag(MethodFlagMap);
            }
            else if (word == "optional")
            {
                ret->options.emplace_back(readValue());
            }
            else if (word == "paramname")
            {
                ret->paramNames.emplace_back(readString());
            }
            else if (word == "body")
            {
                ret->vbody         = readMethodBody();
                ret->vbody->method = ret;
            }
            else if (word == "end")
            {
                return ret;
            }
            else
            {
                throw StringException("Unknown method field " + word);
            }
        }
    }

    ASASM::Instance readInstance()
    {
        ASASM::Instance ret;
        ret.name = readMultiname();
        while (true)
        {
            std::string word = readWord();
            if (word == "extends")
            {
                mustBeNull(ret.superName);
                ret.superName = readMultiname();
            }
            else if (word == "implements")
            {
                ret.interfaces.emplace_back(readMultiname());
            }
            else if (word == "flag")
            {
                ret.flags |= readFlag(InstanceFlagMap);
            }
            else if (word == "protectedns")
            {
                mustBeNull(ret.protectedNs);
                ret.protectedNs = readNamespace();
            }
            else if (word == "iinit")
            {
                mustBeNull(ret.iinit);
                ret.iinit = readMethod();
            }
            else if (word == "trait")
            {
                ret.traits.emplace_back(readTrait());
            }
            else if (word == "end")
            {
                mustBeSet("iinit", ret.iinit);
                return ret;
            }
            else
            {
                throw StringException("Unknown instance field " + word);
            }
        }
    }

    std::shared_ptr<ASASM::Class> readClass()
    {
        auto ret = std::make_shared<ASASM::Class>();
        while (true)
        {
            std::string word = readWord();
            if (word == "refid")
            {
                addUnique<addUniqueClass>(classesByID, readString(), ret);
            }
            else if (word == "instance")
            {
                mustBeNull(ret->instance);
                ret->instance = readInstance();
            }
            else if (word == "cinit")
            {
                mustBeNull(ret->cinit);
                ret->cinit = readMethod();
            }
            else if (word == "trait")
            {
                ret->traits.emplace_back(readTrait());
            }
            else if (word == "end")
            {
                mustBeSet("cinit", ret->cinit);
                mustBeSet("instance", ret->instance);
                return ret;
            }
            else
            {
                throw StringException("Unknown class field " + word);
            }
        }
    }

    std::shared_ptr<ASASM::Script> readScript()
    {
        std::shared_ptr<ASASM::Script> ret(new ASASM::Script);
        while (true)
        {
            std::string word = readWord();
            if (word == "sinit")
            {
                mustBeNull(ret->sinit);
                ret->sinit = readMethod();
            }
            else if (word == "trait")
            {
                ret->traits.emplace_back(readTrait());
            }
            else if (word == "end")
            {
                static constexpr char sinit[] = "sinit";
                mustBeSet("sinit", ret->sinit);
                return ret;
            }
            else
            {
                backpedal(word.size());
                throw StringException("Unknown script field " + word);
            }
        }
    }

    ASASM::MethodBody readMethodBody()
    {
        std::unordered_map<std::string, uint32_t> labels;
        ASASM::MethodBody ret;
        while (true)
        {
            std::string word = readWord();
            if (word == "maxstack")
            {
                ret.maxStack = std::max<uint32_t>(ret.maxStack, (uint32_t)readUInt());
            }
            else if (word == "localcount")
            {
                ret.localCount = std::max<uint32_t>(ret.localCount, (uint32_t)readUInt());
            }
            else if (word == "initscopedepth")
            {
                ret.initScopeDepth = (uint32_t)readUInt();
            }
            else if (word == "maxscopedepth")
            {
                ret.maxScopeDepth = (uint32_t)readUInt();
            }
            else if (word == "code")
            {
                ret.instructions = readInstructions(labels);
            }
            else if (word == "try")
            {
                ret.exceptions.emplace_back(readException(labels));
            }
            else if (word == "trait")
            {
                ret.traits.emplace_back(readTrait());
            }
            else if (word == "end")
            {
                return ret;
            }
            else
            {
                backpedal(word.size());
                throw StringException("Unknown method field " + word);
            }
        }
    }

    SWFABC::Label parseLabel(
        const std::string& label, const std::unordered_map<std::string, uint32_t>& labels)
    {
        std::string name = label;
        int offset       = 0;
        for (size_t i = 0; i < label.size(); i++)
        {
            if (label[i] == '-' || label[i] == '+')
            {
                name   = label.substr(0, i);
                offset = atoi(label.c_str() + i + 1);
                if (label[i] == '-')
                {
                    offset = -offset;
                }
                break;
            }
        }

        if (!labels.contains(name))
        {
            backpedal(label.size());
            throw StringException("Unknown label " + name);
        }

        return SWFABC::Label{labels.at(name), offset, 0};
    }

    std::vector<ASASM::Instruction> readInstructions(
        std::unordered_map<std::string, uint32_t>& _labels)
    {
        std::vector<ASASM::Instruction> ret;

        struct LocalFixup
        {
            Position where;
            uint32_t ii, ai;
            std::string name;
            uint32_t si;
        };

        std::vector<LocalFixup> jumpFixups, switchFixups, localClassFixups, localMethodFixups;
        std::unordered_map<std::string, uint32_t> labels;

        while (true)
        {
            std::string word = readWord();
            if (word == "end")
            {
                break;
            }
            if (peekChar() == ':')
            {
                addUnique<addUniqueLabel>(labels, word, uint32_t(ret.size()));
                skipChar();
                continue;
            }

            auto opcode = OPCodeMap.Find(word.c_str());
            if (!opcode)
            {
                backpedal(word.size());
                throw StringException("Unknown OPCode " + word);
            }

            ASASM::Instruction instruction;
            instruction.opcode = *opcode;
            instruction.arguments.resize(OPCode_Info[(uint8_t)instruction.opcode].second.size());
            for (size_t i = 0; i < OPCode_Info[(uint8_t)instruction.opcode].second.size(); i++)
            {
                switch (OPCode_Info[(uint8_t)instruction.opcode].second[i])
                {
                    case OPCodeArgumentType::Unknown:
                        throw StringException(
                            "Don't know how to assemble OP_" +
                            std::string(OPCode_Info[(uint8_t)instruction.opcode].first));

                    case OPCodeArgumentType::ByteLiteral:
                        if (sourceVersion < 4)
                        {
                            instruction.arguments[i].bytev((uint8_t)readInt());
                        }
                        else
                        {
                            instruction.arguments[i].bytev((int8_t)readInt());
                        }
                        break;
                    case OPCodeArgumentType::UByteLiteral:
                        instruction.arguments[i].ubytev((uint8_t)readUInt());
                        break;
                    case OPCodeArgumentType::IntLiteral:
                        instruction.arguments[i].intv(readInt());
                        break;
                    case OPCodeArgumentType::UIntLiteral:
                        instruction.arguments[i].uintv(readUInt());
                        break;

                    case OPCodeArgumentType::Int:
                        instruction.arguments[i].intv(readInt());
                        break;
                    case OPCodeArgumentType::UInt:
                        instruction.arguments[i].uintv(readUInt());
                        break;
                    case OPCodeArgumentType::Double:
                        instruction.arguments[i].doublev(readDouble());
                        break;
                    case OPCodeArgumentType::String:
                        instruction.arguments[i].stringv(readString());
                        break;
                    case OPCodeArgumentType::Namespace:
                        instruction.arguments[i].namespacev(readNamespace());
                        break;
                    case OPCodeArgumentType::Multiname:
                        instruction.arguments[i].multinamev(readMultiname());
                        break;
                    case OPCodeArgumentType::Class:
                        instruction.arguments[i].classv({});
                        localClassFixups.emplace_back(
                            currentFile->position(), ret.size(), i, readString(), 0);
                        break;
                    case OPCodeArgumentType::Method:
                        instruction.arguments[i].methodv({});
                        localMethodFixups.emplace_back(
                            currentFile->position(), ret.size(), i, readString(), 0);
                        break;

                    case OPCodeArgumentType::JumpTarget:
                    case OPCodeArgumentType::SwitchDefaultTarget:
                        instruction.arguments[i].jumpTarget({});
                        jumpFixups.emplace_back(
                            currentFile->position(), ret.size(), i, readWord(), 0);
                        break;

                    case OPCodeArgumentType::SwitchTargets:
                    {
                        std::vector<std::string> switchTargetLabels =
                            readList<'[', ']', false>([this] { return readWord(); });
                        instruction.arguments[i].switchTargets(
                            std::vector<SWFABC::Label>(switchTargetLabels.size()));
                        for (size_t li = 0; li < switchTargetLabels.size(); li++)
                        {
                            switchFixups.emplace_back(
                                currentFile->position(), ret.size(), i, switchTargetLabels[li], li);
                        }
                    }
                    break;
                }
                if (i + 1 < OPCode_Info[(uint8_t)instruction.opcode].second.size())
                {
                    expectSymbol(',');
                }
            }
            if (instruction.opcode == OPCode::OP_getlocal && instruction.arguments[0].uintv() < 4)
            {
                instruction.opcode =
                    OPCode(uint8_t(OPCode::OP_getlocal0) + instruction.arguments[0].uintv());
            }
            else if (instruction.opcode == OPCode::OP_setlocal &&
                     instruction.arguments[0].uintv() < 4)
            {
                instruction.opcode =
                    OPCode(uint8_t(OPCode::OP_setlocal0) + instruction.arguments[0].uintv());
            }

            if (includeDebugInstructions || (instruction.opcode != OPCode::OP_debug &&
                                                instruction.opcode != OPCode::OP_debugfile &&
                                                instruction.opcode != OPCode::OP_debugline))
            {
                ret.emplace_back(std::move(instruction));
            }
        }

        for (const auto& f : jumpFixups)
        {
            try
            {
                ret[f.ii].arguments[f.ai].jumpTarget(parseLabel(f.name, labels));
            }
            catch (std::exception& e)
            {
                setFile(f.where.load());
                throw e;
            }
        }

        for (const auto& f : switchFixups)
        {
            try
            {
                ret[f.ii].arguments[f.ai].switchTargets()[f.si] = parseLabel(f.name, labels);
            }
            catch (std::exception& e)
            {
                setFile(f.where.load());
                throw e;
            }
        }

        for (const auto& f : localClassFixups)
        {
            classFixups.emplace_back(f.where, ret[f.ii].arguments[f.ai].classv(), f.name);
        }
        for (const auto& f : localMethodFixups)
        {
            methodFixups.emplace_back(f.where, ret[f.ii].arguments[f.ai].methodv(), f.name);
        }

        _labels = labels;
        return ret;
    }

    ASASM::Exception readException(const std::unordered_map<std::string, uint32_t>& labels)
    {
        auto readLabel = [this, &labels]
        {
            std::string word = readWord();
            try
            {
                return parseLabel(word, labels);
            }
            catch (std::exception& e)
            {
                backpedal(word.size());
                throw e;
            }
        };

        ASASM::Exception ret;
        while (true)
        {
            std::string word = readWord();
            if (word == "from")
            {
                ret.from = readLabel();
            }
            else if (word == "to")
            {
                ret.to = readLabel();
            }
            else if (word == "target")
            {
                ret.target = readLabel();
            }
            else if (word == "type")
            {
                ret.excType = readMultiname();
            }
            else if (word == "name")
            {
                ret.varName = readMultiname();
            }
            else if (word == "end")
            {
                return ret;
            }
            else
            {
                throw StringException("Unknown exception field " + word);
            }
        }
    }

    Assembler(
        const std::unordered_map<std::string, std::string>& strings, bool includeDebugInstructions)
        : strings(strings), includeDebugInstructions(includeDebugInstructions)
    {
    }

    ASASM::ASProgram readProgram()
    {
        ASASM::ASProgram ret;
        expectWord("program");
        while (true)
        {
            std::string word = readWord();
            if (word == "minorversion")
            {
                ret.minorVersion = (uint16_t)readUInt();
            }
            else if (word == "majorversion")
            {
                ret.majorVersion = (uint16_t)readUInt();
            }
            else if (word == "script")
            {
                ret.scripts.emplace_back(readScript());
            }
            else if (word == "class")
            {
                ret.orphanClasses.emplace_back(readClass());
            }
            else if (word == "method")
            {
                ret.orphanMethods.emplace_back(readMethod());
            }
            else if (word == "end")
            {
                return ret;
            }
            else
            {
                backpedal(word.size());
                throw StringException("Unknown program field " + word);
            }
        }
    }

    std::string context()
    {
        std::string ret = currentFile->positionStr() + ": ";
        for (std::shared_ptr<SourceFile> f = currentFile->parent; f != nullptr; f = f->parent)
        {
            ret += "\n\t(included from " + f->positionStr() + ")";
        }
        return ret;
    }

    void applyFixups()
    {
        for (const auto& f : classFixups)
        {
            if (f.name.empty())
            {
                f.ptr = nullptr;
            }
            else
            {
                if (!classesByID.contains(f.name))
                {
                    setFile(f.where.load());
                    throw StringException("Unknown class refid: " + f.name);
                }
                f.ptr = classesByID.at(f.name);
            }
        }

        for (const auto& f : methodFixups)
        {
            if (f.name.empty())
            {
                f.ptr = nullptr;
            }
            else
            {
                if (!methodsByID.contains(f.name))
                {
                    setFile(f.where.load());
                    throw StringException("Unknown class refid: " + f.name);
                }
                f.ptr = methodsByID.at(f.name);
            }
        }

        classFixups.clear();
        methodFixups.clear();
        classesByID.clear();
        methodsByID.clear();
    }

public:
    static ASASM::ASProgram assemble(
        const std::unordered_map<std::string, std::string>& strings, bool includeDebugInstructions)
    {
        if (!strings.contains("main.asasm"))
        {
            throw StringException("Assembly start (main.asasm) not found");
        }

        Assembler assembler(strings, includeDebugInstructions);

        std::shared_ptr<SourceFile> mainFile =
            std::make_shared<SourceFile>("main.asasm", strings.at("main.asasm"));
        assembler.pushFile(mainFile);

        try
        {
            ASASM::ASProgram ret = assembler.readProgram();
            assembler.applyFixups();
            return ret;
        }
        catch (std::exception& e)
        {
            throw StringException("\n" + assembler.context() + "\n" + e.what());
        }
    }
};
