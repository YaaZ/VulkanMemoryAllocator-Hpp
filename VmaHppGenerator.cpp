#include <algorithm>
#include <array>
#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

using Match = std::match_results<std::string_view::const_iterator>;
template<class Iterator> class Iterable {
    Iterator b, e;
public:
    explicit Iterable(Iterator  i) : b(std::move(i)), e() {}
    const Iterator& begin() { return b; }
    const Iterator& end() { return e; }
};
auto iterate(const std::string_view& string, const std::regex& pattern) {
    return Iterable(std::regex_iterator(string.begin(), string.end(), pattern));
}
std::string_view group(const std::string_view& source, const Match& match, const std::size_t group = 0) {
    return source.substr(match.position(group), match.length(group));
}
bool startsWith(const std::string_view string, const std::string_view prefix) {
    return string.length() >= prefix.length() && string.substr(0, prefix.length()) == prefix;
}
bool endsWith(const std::string_view string, const std::string_view suffix) {
    return string.length() >= suffix.length() && string.substr(string.length() - suffix.length()) == suffix;
}
std::string_view trim(std::string_view string) {
    while (!string.empty() && std::isspace(string.front())) string.remove_prefix(1);
    while (!string.empty() && std::isspace(string.back())) string.remove_suffix(1);
    return string;
}
char capitalize(const char c, const bool upper) {
    return static_cast<char>(upper ? std::toupper(c) : std::tolower(c));
}

/**
 * Node-based text representation for template processing and code generation.
 */
class Segment;
Segment operator ""_seg(const char* data, std::size_t size);
class ConditionalTree;
class Segment {
    struct BreakNode { int count; };
    struct NavigateNode { int sourcePosition; };
    struct WildcardNode {
        int index, indent;
    };
    struct StringViewNode {
        const char* data;
        int length, indent;
    };
    struct StringNode {
        struct Data {
            std::string string;
            int indent;
        };
        std::shared_ptr<Data> data;
    };
    using Node = std::variant<BreakNode, NavigateNode, WildcardNode, StringViewNode, StringNode>;

    template<class T, std::size_t> using ArrayTupleElement = T;
    template<class T, std::size_t... I>
    static auto ArrayTupleType(std::index_sequence<I...>) -> std::tuple<ArrayTupleElement<T, I>...>* { return nullptr; }
    template<class T, std::size_t S> using ArrayTuple = std::remove_pointer_t<decltype(ArrayTupleType<T>(std::make_index_sequence<S>()))>;

public:
    template<int Size, class T = Segment> class Vector : public ArrayTuple<T, Size> {
        friend class Segment;
        template<std::size_t... I> Vector<Size + 1, T> join(Segment& o, std::index_sequence<I...>) && {
            return { std::get<I>(*this)..., o };
        }
        template<bool Back, class Arg, std::size_t I = 0> void plus(const Arg& arg) {
            if constexpr (Back) std::get<I>(*this) + arg;
            else arg + std::get<I>(*this);
            if constexpr (I < Size - 1) plus<Back, Arg, I + 1>(arg);
        }
    public:
        using ArrayTuple<T, Size>::ArrayTuple;
        auto& operator*() { return static_cast<ArrayTuple<T, Size>&>(*this); }
        Vector<Size + 1, T> operator,(Segment& o) && { return std::move(*this).join(o, std::make_index_sequence<Size>()); }
        template<class Arg> friend Vector& operator+(Vector& v, const Arg& arg) { v.template plus<true>(arg); return v; }
        template<class Arg> friend Vector& operator+(const Arg& arg, Vector& v) { v.template plus<false>(arg); return v; }
        template<class Arg> friend Vector&& operator+(Vector&& v, const Arg& arg) { v.template plus<true>(arg); return std::move(v); }
        template<class Arg> friend Vector&& operator+(const Arg& arg, Vector&& v) { v.template plus<false>(arg); return std::move(v); }
    };

    class Token : public Node {
    public:
        Token() : Node(StringViewNode { nullptr, 0, 0 }) {}
        Token(const char* s, const int indent = 0) : Node(StringViewNode { s, static_cast<int>(std::strlen(s)), indent }) {}
        template<std::size_t N> Token(const char(&s)[N], int indent = 0) : Node(StringViewNode { s, N - 1, indent }) {}
        Token(const std::string_view& s, const int indent = 0) : Node(StringViewNode { s.data(), static_cast<int>(s.length()), indent }) {}
        Token(const std::string& s, const int indent = 0) : Node(StringNode { std::make_shared<StringNode::Data>(StringNode::Data { s, indent }) }) {}
        explicit operator std::string() const { return std::string(**this); }
        std::size_t length() const { return (**this).length(); }
        explicit operator bool() const { return length() > 0; }
        std::string_view operator*() const {
            if (const auto* n = std::get_if<StringViewNode>(this))
                return { n->data, static_cast<unsigned int>(n->length) };
            return std::get<StringNode>(*this).data->string;
        }
        Token substr(const std::size_t pos = 0, const std::size_t count = -1) const {
            if (const auto* n = std::get_if<StringViewNode>(this))
                return Token(std::string_view(n->data + pos, std::min(n->length - pos, count)), n->indent);
            const auto& [string, indent] = *std::get<StringNode>(*this).data;
            return Token(string.substr(pos, std::min(string.length() - pos, count)), indent);
        }
        Token capitalize(const bool upper = true) const {
            if (length() > 0) {
                if (const char o = (**this)[0], n = ::capitalize(o, upper); o != n) {
                    std::string s { *this };
                    s.front() = n;
                    return s;
                }
            }
            return *this;
        }
    };

private:
    static std::unordered_map<const char*, const Segment> cache;
    std::vector<Node> nodes;

    Segment(const char* data, const std::size_t size) { // Cached constructor.
        if (const auto cached = cache.find(data); cached == cache.end()) {
            parse({ data, size });
            cache.emplace(data, *this);
        } else *this = cached->second;
    }

    int parse(std::string_view string) {
        int literalIndent = 0; // Pretty indent for raw string literals.
        for (int i = static_cast<int>(string.length()) - 1; i >= 0; --i) {
            if (string[i] == ' ') continue;
            if (string[i] == '\n') string.remove_suffix(literalIndent = static_cast<int>(string.length()) - 1 - i);
            break;
        }

        auto at = [&](const int i) {
            return i >= 0 && i < string.length() ? string[i] : '\0';
        };

        for (int indent = -literalIndent, i = 0;;) {
            switch (at(i)) {
                case '\n':
                    for (int breaks = i >= 0, j = i + 1;; ++j) {
                        if (const char c = at(j); c == '\n') {
                            i = j;
                            breaks++;
                        } else if (c != ' ' && c != '\t') {
                            indent = j - i - literalIndent - 1;
                            i = j;
                            if (breaks > 0) nodes.emplace_back(BreakNode { breaks });
                            break;
                        }
                    }
                    break;
                case '$':
                    if (const int n = at(i + 1) - '0'; n >= 0 && n <= 9) {
                        nodes.emplace_back(WildcardNode { n, indent });
                        i += 2;
                    } else throw std::runtime_error("Malformed pattern: " + std::string(string.substr(i, 2)));
                    break;
                case '#': // Force preprocessor directives to ignore indent.
                    indent = std::numeric_limits<int>::min();
                    [[fallthrough]];
                default:
                    for (int j = i + 1;; ++j) {
                        if (const char c = at(j); c == '\n' || c == '\0' || c == '$') {
                            nodes.emplace_back(StringViewNode { string.data() + i, j - i, indent });
                            indent += j - i;
                            i = j;
                            break;
                        }
                    }
                    break;
                case '\0':
                    return i;
            }
        }
    }

    static void indentNode(Node& t, const int indent) {
        if (auto* n = std::get_if<WildcardNode>(&t)) n->indent += indent;
        else if (auto* n = std::get_if<StringViewNode>(&t)) n->indent += indent;
        else if (auto* n = std::get_if<StringNode>(&t)) n->data = std::make_shared<StringNode::Data>(
            StringNode::Data { n->data->string, n->data->indent + indent });
    }
    int replaceNode(const int i, const int indent, const Token& replacement) {
        indentNode(nodes[i] = replacement, indent);
        return i;
    }
    int replaceNode(const int i, const int indent, const Segment& replacement) {
        if (!replacement.nodes.empty()) {
            nodes[i] = replacement.nodes[0];
            if (replacement.nodes.size() > 1)
                nodes.insert(nodes.begin() + i + 1, replacement.nodes.begin() + 1, replacement.nodes.end());
            for (int j = 0; j < replacement.nodes.size(); ++j) indentNode(nodes[i + j], indent);
        } else nodes.erase(nodes.begin() + i);
        return i + replacement.nodes.size() - 1;
    }

    template<class T>
    void replaceWildcards(const int index, const T& replacement) {
        for (int i = 0; i < nodes.size(); ++i)
            if (const auto* n = std::get_if<WildcardNode>(&nodes[i]))
                if (n->index == index) i  = replaceNode(i, n->indent, replacement);
    }

    void replaceImpl(const int) {}
    template<class... Args>
    void replaceImpl(const int index, const Token& replacement, const Args&... args) {
        replaceWildcards(index, replacement);
        replaceImpl(index + 1, args...);
    }
    template<class... Args>
    void replaceImpl(const int index, const Segment& replacement, const Args&... args) {
        replaceWildcards(index, replacement);
        replaceImpl(index + 1, args...);
    }

public:
    using Break = BreakNode;
    using Navigate = NavigateNode;

    Segment() = default;
    Segment(const Token& t) { nodes.emplace_back(t); }

    template<std::size_t N> explicit Segment(const char(&source)[N]) : Segment(source, N - 1) {}
    friend Segment operator ""_seg(const char* data, const std::size_t size) { return Segment(data, size); }

    Segment& clear() { nodes.clear(); return *this; }
    Segment& pop() { nodes.pop_back(); return *this; }
    /**
     * Has no text or wildcard tokens (navigation and breaks are ignored).
     */
    bool blank() const {
        for (const auto& n : nodes)
            if (std::holds_alternative<WildcardNode>(n) ||
                std::holds_alternative<StringViewNode>(n) ||
                std::holds_alternative<StringNode>(n)) return false;
        return true;
    }

    Vector<2, Segment&> operator,(Segment& o) { return { *this, o }; }
    friend Segment& operator+(Segment& s, const Token& o) { s.nodes.emplace_back(o); return s; }
    friend Segment& operator+(const Token& o, Segment& s) { s.nodes.emplace(s.nodes.begin(), o); return s; }
    friend Segment& operator+(Segment& s, const Node& o) { s.nodes.emplace_back(o); return s; }
    friend Segment& operator+(const Node& o, Segment& s) { s.nodes.emplace(s.nodes.begin(), o); return s; }

    friend Segment& operator+(Segment& s, Segment&& o) {
        s.nodes.reserve(s.nodes.size() + o.nodes.size());
        std::move(o.nodes.begin(), o.nodes.end(), std::back_inserter(s.nodes));
        return s;
    }
    friend Segment& operator+(Segment&& o, Segment& s) {
        s.nodes.reserve(s.nodes.size() + o.nodes.size());
        std::move(o.nodes.begin(), o.nodes.end(), std::inserter(s.nodes, s.nodes.begin()));
        return s;
    }

    template<class... Args> Segment&& replace(const Args&... args) && { replaceImpl(0, args...); return std::move(*this); }
    template<class... Args> Segment& replace(const Args&... args) & { replaceImpl(0, args...); return *this; }

    Segment& resolve(const ConditionalTree& tree, int startPosition = 0);

    void generateHpp(std::string name) const {
        const std::string file = "vk_mem_alloc_" + name + ".hpp";
        std::transform(name.begin(), name.end(), name.begin(), &toupper);
        R"(// Generated from the Vulkan Memory Allocator (vk_mem_alloc.h).
        #ifndef VULKAN_MEMORY_ALLOCATOR_$0_HPP
        #define VULKAN_MEMORY_ALLOCATOR_$0_HPP

        $1
        #endif
        )"_seg.replace(Token(name), *this).generate(file);
    }

    void generate(const std::string& file) const {
        std::ofstream out(BASE_PATH "/include/" + file);
        out.fill(' ');
        int breaks = 0, length = 0;
        for (const auto& node : nodes) {
            if (const auto* n = std::get_if<BreakNode>(&node)) {
                if (breaks < n->count) breaks = n->count;
                length = 0;
            } else {
                for (; breaks > 0; --breaks) out << '\n';
                std::string_view string;
                int indent = 0;
                if (const auto* n = std::get_if<StringViewNode>(&node)) {
                    string = { n->data, static_cast<unsigned int>(n->length) };
                    indent = n->indent;
                } else if (const auto* n = std::get_if<StringNode>(&node)) {
                    string = n->data->string;
                    indent = n->data->indent;
                } else throw std::runtime_error("Unexpanded node");
                if (indent > length) {
                    out << std::setw(indent - length) << "";
                    length += indent - length;
                }
                length += string.length();
                out << string;
            }
        }
        // Trailing breaks are ignored.
    }
};
std::unordered_map<const char*, const Segment> Segment::cache;

using Token = Segment::Token;
constexpr Segment::Break n { 1 }, nn { 2 };
class {
    template<class T> static int sourcePosition(const T& t) { return static_cast<int>(t.sourcePosition); }
    static int sourcePosition(const long long int sourcePosition) { return static_cast<int>(sourcePosition); }
    static int sourcePosition(const int sourcePosition) { return sourcePosition; }
    static int sourcePosition(const Match& match) { return static_cast<int>(match.position()); }
public:
    const Segment::Navigate reset { -1 };
    template<class... T> Segment::Navigate operator()(const T&... t) const { return { (sourcePosition(t) + ...) }; }
} constexpr navigate;

/**
* Tree of conditional preprocessing (#if - #elif - #else - #endif) blocks.
*/
class ConditionalTree {
    struct Statement {
        std::string_view directive;
        int sourcePosition;
        enum class Type { IF, ELIF, ELSE, ENDIF } type;
        int descent = -1; // Descent chain: $parent <- #if <- #elif <- #else <- #endif
        int ascent = -1; // Ascent chain: #if -> #elif -> #else -> #endif -> $parent
    };

    std::vector<Statement> statements;
public:
    explicit ConditionalTree(const std::string_view source) {
        static const std::unordered_set<std::string_view> ignoredDirectives {
            "ifndef AMD_VULKAN_MEMORY_ALLOCATOR_H",
            "ifndef _VMA_ENUM_DECLARATIONS",
            "ifndef _VMA_DATA_TYPES_DECLARATIONS",
        };

        statements.push_back(Statement { source, std::numeric_limits<int>::min(), Statement::Type::IF });
        std::vector<int> stack;
        stack.push_back(0);
        auto findTop = [&](const int skip = 0) {
            for (int i = stack.size() - skip - 1;; --i) if (stack[i] != -1) return stack[i];
        };

        // Find directives.
        const std::regex pattern("# *(.+)");
        for (const auto& match : iterate(source, pattern)) {
            std::string_view directive = group(source, match, 1);
            Statement::Type type;
            if      (startsWith(directive, "if"))    type = Statement::Type::IF;
            else if (startsWith(directive, "elif"))  type = Statement::Type::ELIF;
            else if (startsWith(directive, "else"))  type = Statement::Type::ELSE;
            else if (startsWith(directive, "endif")) type = Statement::Type::ENDIF;
            else continue;

            // Filter ignored sequences.
            const int added = statements.size(), top = findTop();
            if (type == Statement::Type::IF)
                stack.push_back(ignoredDirectives.find(directive) == ignoredDirectives.end() ? added : -1);

            if (stack.back() != -1) {
                statements.push_back(Statement { directive, static_cast<int>(match.position()), type, top });
                if (type != Statement::Type::IF) {
                    stack.back() = statements[top].ascent = added;
                    if (type == Statement::Type::ENDIF) statements.back().ascent = findTop(1);
                }
            }

            if (type == Statement::Type::ENDIF) stack.pop_back();
        }

        // Close hanging blocks (at least the root one).
        for (; !stack.empty(); stack.pop_back()) {
            if (stack.back() != -1) {
                const int top = findTop();
                statements[top].ascent = statements.size();
                statements.push_back(Statement { "endif", std::numeric_limits<int>::max(), Statement::Type::ENDIF,
                    top, stack.size() == 1 ? -1 : findTop(1) });
            }
        }
    }

    class Traversal {
        friend class ConditionalTree;
        Segment ascent, descent;
        const ConditionalTree* tree;
        int statement;
        explicit Traversal(const ConditionalTree* tree, const int statement) : tree(tree), statement(statement) {}

        const Statement& at(const int i) const { return tree->statements[i]; }
        const Statement& current() const { return at(statement); }

    public:
        const Segment& navigate(const int sourcePosition) {
            ascent.clear();
            descent.clear();
            // Ascend.
            for (int from, to, next;
                sourcePosition >= (to = at(next = current().ascent).sourcePosition) |
                sourcePosition < (from = current().sourcePosition); statement = next)
                if (to > from) ascent + n + "#" + at(next).directive;
            // Find target statement.
            int target;
            for (int from = statement, to = current().ascent; (target = (from + to) / 2) > from;)
                (sourcePosition < at(target).sourcePosition ? to : from) = target;
            while (at(target).type == Statement::Type::ENDIF) target = at(target).ascent;
            // Descend into the target (ascend from the target, actually).
            for (int i = target; i != statement; i = at(i).descent)
                n + ("#" + (at(i).directive + descent));
            statement = target;
            return ascent.blank() && descent.blank() ? ascent : ascent + std::move(descent) + n;
        }
    };
    Traversal traverse(const int start = 0) const { return Traversal(this, start); }
};

Segment& Segment::resolve(const ConditionalTree& tree, const int startPosition) {
    auto traverse = tree.traverse(startPosition);
    for (int i = 0; i < nodes.size(); ++i)
        if (const auto* n = std::get_if<NavigateNode>(&nodes[i]))
            i  = replaceNode(i, std::numeric_limits<int>::min(), traverse.navigate(n->sourcePosition));
    return *this;
}

/**
 * Combined source string and conditional tree.
 */
struct Source : std::string_view {
    ConditionalTree tree;
    explicit Source(const std::string& source) : std::string_view(source), tree(std::string_view(source)) {}
};

struct Symbol {
    Token name;
    Segment::Navigate navigate;
    Symbol(const Symbol&) = default;
    template<class... T> explicit Symbol(Token name, const T&... t) : name(std::move(name)), navigate(::navigate(t...)) {}
};
using Symbols = std::vector<Symbol>;

/**
 * Parsed variable declaration (field or function parameter).
 */
class Var {
public:
    Token name;
    Token type; // Full converted C++ type
    Token underlyingType; // Original type without array size, const and pointers
    Token lenIfNotNull;
    enum class Kind : std::uint8_t { OTHER, VOID, CHAR, PFN, VK, VMA, ARRAY } kind;
    enum class Tag : std::uint8_t { NONE, NULLABLE, NOT_NULL } tag;
    std::uint8_t pointers;
    bool constant;
    int sourcePosition;

private:
    static constexpr std::string_view pattern =
        R"((const\s+)?(\w+))"
        R"((\s*\*)?(?:\s+VMA_(NULLABLE|NOT_NULL)(?:_NON_DISPATCHABLE)?)?)"
        R"((\s*\*)?(?:\s+VMA_(NULLABLE|NOT_NULL)(?:_NON_DISPATCHABLE)?)?)"
        R"((?:\s+(VMA_LEN_IF_NOT_NULL|VMA_EXTENDS_VK_STRUCT)\(\s*([^)]+)\s*\))?)"
        R"(\s+(\w+))"
        R"(((?:\s*\[\s*\w+\s*\])+)?)"
        R"(\s*[;,)])";

    Var(const std::string_view& source, const Match& match, const int sourcePosition) : sourcePosition(sourcePosition) {
        auto group = [&](const int i) { return ::group(source, match, i); };
        name = group(9);
        underlyingType = group(2);

        std::string type { underlyingType };
        if (type == "uint32_t" || type == "size_t" || type == "float" || type == "HANDLE") kind = Kind::OTHER;
        else if (type == "void") kind = Kind::VOID;
        else if (type == "char") kind = Kind::CHAR;
        else if (startsWith(type, "PFN_")) kind = Kind::PFN;
        else if (startsWith(type, "Vk")) {
            kind = Kind::VK;
            type = "VULKAN_HPP_NAMESPACE::" + type.substr(2);
        } else if (startsWith(type, "Vma")) {
            kind = Kind::VMA;
            type = type.substr(3);
        } else throw std::runtime_error("Unknown type: " + type);

        constant = !group(1).empty();
        const bool p1 = !group(3).empty(), p2 = !group(5).empty();
        pointers = p1 + p2;
        if (constant && pointers > 0) type = "const " + type;
        if (pointers == 2) {
            constant = false; // double pointer, then first-level pointer is not const
            type += "**";
        } else if (pointers == 1) type += "*";

        std::string_view arr = group(10);
        while (!(arr = trim(arr)).empty()) {
            const auto i = arr.find_last_of('['); // assert(i != npos);
            std::string dim { trim(arr.substr(i + 1, arr.length() - i - 2)) };
            type = "std::array<" + type + ", " + dim + ">";
            arr = arr.substr(0, i);
            kind = Kind::ARRAY;
        }
        this->type = type;

        const std::string_view tag = p2 ? group(6) : group(4);
        if (tag == "NULLABLE") this->tag = Tag::NULLABLE;
        else if (tag == "NOT_NULL") this->tag = Tag::NOT_NULL;
        else this->tag = Tag::NONE;

        if (group(7) == "VMA_LEN_IF_NOT_NULL") lenIfNotNull = group(8);
    }

public:

    static std::vector<Var> parse(const std::string_view& source, const long long int sourcePositionOffset = 0) {
        static std::regex pattern { Var::pattern.data(), Var::pattern.length() };
        std::vector<Var> result;
        for (const auto& match : iterate(source, pattern))
            result.push_back(Var(source, match, static_cast<int>(match.position() + sourcePositionOffset)));
        return result;
    }
};

Symbols generateEnums(const Source& source) {
    Symbols enums;
    const std::regex typedefPattern { R"(typedef\s+enum\s+Vma(\w+))" };
    const std::regex entryPattern { R"((VMA_\w+)[^,}]*)" };

    Segment::Vector<4> enumPieces;
    auto& [content, toString, flagTraits, operators] = *enumPieces;
    for (const auto& match : iterate(source, typedefPattern)) {
        Token name = group(source, match, 1); // E.g. AllocationCreateFlagBits
        Token flagBits;                       // E.g. AllocationCreateFlags
        if (endsWith(*name, "FlagBits")) flagBits = std::string(name.substr(0, name.length() - 4)) + "s";
        Token baseName = flagBits ? name.substr(0, name.length() - 8) : name; // E.g. AllocationCreate
        enums.emplace_back(name, match);
        if (flagBits) enums.emplace_back(flagBits, match);

        Segment::Vector<4> entryPieces;
        auto& [entries, entryToString, allFlags, flagsToString] = *entryPieces;
        Segment declaration = name;
        if (flagBits) declaration + " : Vma" + flagBits;

        const auto begin = match.position() + match.length();
        const auto body = source.substr(begin, source.find("}", begin) - begin);
        for (const auto& entryMatch : iterate(body, entryPattern)) {
            auto originalEntry = group(body, entryMatch, 1);
            if (endsWith(originalEntry, "_MAX_ENUM")) break;
            bool bitEntry = endsWith(originalEntry, "_BIT"); // assert(!bitEntry || flagBits)

            Token entry = [&] {
                std::string e;
                // Convert to CamelCase.
                e.reserve(originalEntry.length()); // Not precise.
                for (int i = /*VMA_*/4, u = 1; i < originalEntry.length() - (bitEntry ? 4/*_BIT*/ : 0); ++i) {
                    if (const char c = originalEntry[i]; c != '_') {
                        e += capitalize(c, u);
                        u = 0;
                    } else u = 1;
                }
                // Strip base enum name.
                if (!startsWith(e, *baseName)) throw std::runtime_error("Unexpected enum value: " + std::string(e));
                e.erase(e.begin(), e.begin() + static_cast<int>(baseName.length()));
                return e;
            }();

            entryPieces + navigate(begin, entryMatch);
            entries + "\ne$0 = $1,"_seg.replace(entry, originalEntry);
            entryToString + "\nif (value == $0::e$1) return \"$1\";"_seg.replace(name, entry);
            if (flagBits && bitEntry) {
                allFlags + "\n$2 VMA_HPP_NAMESPACE::$0::e$1"_seg.replace(name, entry, allFlags.blank() ? " " : "|");
                flagsToString + "\nif (value & $0::e$1) result += \" $1 |\";"_seg.replace(name, entry);
            }
        }
        entries.pop();
        entryPieces + navigate(match);

        (content, toString) + nn + navigate(match);

        content + R"(
        enum class $0 {
          $1
        };
        )"_seg.replace(declaration, entries);
        if (flagBits) content + "using " + flagBits + " = VULKAN_HPP_NAMESPACE::Flags<" + name + ">;";

        toString + R""(
        VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string($0 value) {
          $1
          return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
        }
        )""_seg.replace(name, entryToString);

        if (flagBits) {
            (flagTraits, operators) + nn + navigate(match);

            flagTraits + R"(

            template<> struct FlagTraits<VMA_HPP_NAMESPACE::$0> {
              using WrappedType = Vma$0;
              static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
              static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<VMA_HPP_NAMESPACE::$0> allFlags =
                $1;
            };
            )"_seg.replace(name, allFlags);

            // Operators in vk:: are not found by ADL, so we define them explicitly in vma::
            operators + R"(

            VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR $1 operator|($0 bit0, $0 bit1) VULKAN_HPP_NOEXCEPT {
              return $1(bit0) | bit1;
            }
            VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR $1 operator&($0 bit0, $0 bit1) VULKAN_HPP_NOEXCEPT {
              return $1(bit0) & bit1;
            }
            VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR $1 operator^($0 bit0, $0 bit1) VULKAN_HPP_NOEXCEPT {
              return $1(bit0) ^ bit1;
            }
            VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR $1 operator~($0 bits) VULKAN_HPP_NOEXCEPT {
              return ~$1(bits);
            }
            )"_seg.replace(name, flagBits);

            toString + R"(

            VULKAN_HPP_INLINE std::string to_string($0 value) {
              if (!value) return "{}";
              std::string result = "{";
              $1
              if (result.size() > 1) result.back() = '}';
              else result = "{}";
              return result;
            }
            )"_seg.replace(flagBits, flagsToString);
        }
        entryPieces + navigate.reset;
    }

    R"(
    namespace VMA_HPP_NAMESPACE {
      $0
    }

    namespace VULKAN_HPP_NAMESPACE {
      $1
    }

    namespace VMA_HPP_NAMESPACE {
      $2
    }
    )"_seg.replace(content, flagTraits, operators).resolve(source.tree).generateHpp("enums");
    R"(
    namespace VMA_HPP_NAMESPACE {
      $0
    }
    )"_seg.replace(toString).resolve(source.tree).generateHpp("to_string");
    return enums;
}

Symbols generateStructs(const Source& source) {
    Symbols structs;
    const std::regex typedefPattern { R"(typedef\s+struct\s+Vma(\w+))" };

    Segment content, staticAsserts;
    for (const auto& match : iterate(source, typedefPattern)) {
        Token name = group(source, match, 1); // E.g. DeviceMemoryCallbacks
        structs.emplace_back(name, match);

        const auto begin = match.position() + match.length();
        const auto body = source.substr(begin, source.find("}", begin) - begin);
        std::vector<Var> members = Var::parse(body, begin);

        Token constexprToken = "VULKAN_HPP_CONSTEXPR";
        Segment::Vector<7> memberPieces;
        auto& [constructorParams, constructorInitializer, setters, reflectTypes, reflectTie, comparison, fields] = *memberPieces;
        bool containsPFN = false;
        for (const Var& member : members) {
            Token storageType = member.type;
            if (member.kind == Var::Kind::ARRAY) {
                static const std::regex regex { "std::array" };
                storageType = std::regex_replace(std::string(member.type), regex, "VULKAN_HPP_NAMESPACE::ArrayWrapper1D");
                constexprToken = "VULKAN_HPP_CONSTEXPR_14";
            } else containsPFN |= member.kind == Var::Kind::PFN;
            memberPieces + navigate(member);
            if (!constructorParams.blank())
                (constructorParams, constructorInitializer, reflectTypes, reflectTie) + n + Token(", ", -2);
            constructorParams + "$1 $0_ = {}"_seg.replace(member.name, member.type);
            constructorInitializer + "$0 { $0_ }"_seg.replace(member.name);
            setters + R"(
            VULKAN_HPP_CONSTEXPR_14 $0& set$3($2 $1_) VULKAN_HPP_NOEXCEPT {
              $1 = $1_;
              return *this;
            }

            )"_seg.replace(name, member.name, member.type, member.name.capitalize());
            reflectTypes + storageType + " const &";
            reflectTie + member.name;
            if (!comparison.blank()) comparison + n + Token("&& ", -3);
            comparison + "$0 == rhs.$0"_seg.replace(member.name);
            fields + "\n$1 $0 = {};"_seg.replace(member.name, storageType);
        }
        setters.pop();
        memberPieces + navigate(match);

        comparison = R"(
        bool operator==($0 const & rhs) const VULKAN_HPP_NOEXCEPT {
        #if defined( VULKAN_HPP_USE_REFLECT )
          return this->reflect() == rhs.reflect();
        #else
          return $1;
        #endif
        }
        bool operator!=($0 const & rhs) const VULKAN_HPP_NOEXCEPT {
          return !operator==(rhs);
        }
        )"_seg.replace(name, comparison);
        if (!containsPFN)
            comparison = R"(
            #if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
            auto operator<=>($0 const &) const = default;
            #else
            $1
            #endif
            )"_seg.replace(name, comparison);

        (content, staticAsserts) + nn + navigate(match);

        content + R"(
        struct $0 {
          using NativeType = Vma$0;

        #if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
          $1 $0(
              $2
          ) VULKAN_HPP_NOEXCEPT
            : $3 {}

          $1 $0($0 const &) VULKAN_HPP_NOEXCEPT = default;
          $0(Vma$0 const & rhs) VULKAN_HPP_NOEXCEPT : $0(*reinterpret_cast<$0 const *>(&rhs)) {}

          $0& operator=($0 const &) VULKAN_HPP_NOEXCEPT = default;
        #endif

          $0& operator=(Vma$0 const & rhs) VULKAN_HPP_NOEXCEPT {
            *this = *reinterpret_cast<VMA_HPP_NAMESPACE::$0 const *>(&rhs);
            return *this;
          }

        #if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
          $4
        #endif

          operator Vma$0 const &() const VULKAN_HPP_NOEXCEPT {
            return *reinterpret_cast<const Vma$0 *>(this);
          }

          operator Vma$0&() VULKAN_HPP_NOEXCEPT {
            return *reinterpret_cast<Vma$0 *>(this);
          }

          operator Vma$0 const *() const VULKAN_HPP_NOEXCEPT {
            return reinterpret_cast<const Vma$0 *>(this);
          }

          operator Vma$0*() VULKAN_HPP_NOEXCEPT {
            return reinterpret_cast<Vma$0 *>(this);
          }

        #if defined( VULKAN_HPP_USE_REFLECT )
          std::tuple<$5>
          reflect() const VULKAN_HPP_NOEXCEPT {
            return std::tie($6);
          }
        #endif

        $7

        public:
          $8
        };
        )"_seg.replace(name, constexprToken, constructorParams, constructorInitializer, setters, reflectTypes, reflectTie, comparison, fields);

        staticAsserts + R"(
        VULKAN_HPP_STATIC_ASSERT(sizeof(VMA_HPP_NAMESPACE::$0) == sizeof(Vma$0), "struct and wrapper have different size!");
        VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<VMA_HPP_NAMESPACE::$0>::value, "struct wrapper is not a standard layout!");
        VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<VMA_HPP_NAMESPACE::$0>::value, "$0 is not nothrow_move_constructible!");
        )"_seg.replace(name);

        (content, staticAsserts) + navigate.reset;
    }

    R"(
    namespace VMA_HPP_NAMESPACE {
      $0
    }
    )"_seg.replace(content).resolve(source.tree).generateHpp("structs");
    ("#include <vk_mem_alloc.hpp>" + staticAsserts).resolve(source.tree).generateHpp("static_assertions");
    return structs;
}

std::pair<Symbols, Symbols> generateHandles(const Source& source) {
    Symbols handles, functions;

    struct Handle : Symbol {
        Token lowerName;
        explicit Handle(const Symbol& symbol) : Symbol(symbol), lowerName(symbol.name.capitalize(false)) {}
    } namespaceHandle {Symbol("", -1)};
    std::vector<Handle> handleVector;
    auto findHandle = [&](const Var& var) -> Handle* {
        if (var.kind == Var::Kind::VMA)
            for (Handle& handle : handleVector)
                if (*handle.name == (*var.underlyingType).substr(3)) return &handle;
        return nullptr;
    };

    // Find all handles.
    const std::regex handlePattern { R"(VK_DEFINE_(NON_DISPATCHABLE_)?HANDLE\s*\(\s*(Vma\w+)\s*\))" };
    for (const auto& match : iterate(source, handlePattern)) {
        auto originalName = group(source, match, 2); // E.g. VmaAllocator
        Token name = originalName.substr(3);         // E.g. Allocator
        handles.emplace_back(name, match);
        handleVector.emplace_back(handles.back());
    }

    // Iterate VMA functions.
    const std::regex funcPattern { R"(VMA_CALL_PRE\s+(.+)\s+VMA_CALL_POST\s+(vma\w+)\s*(\([\s\S]+?\)\s*;))" };
    for (const auto& match : iterate(source, funcPattern)) {
        struct Param : Var {
            explicit Param(Var&& v) : Var(std::move(v)) {}
            // TODO Handle name
            // Array params point to the corresponding size param, size param points to itself, rest are -1.
            int deducedArraySize = -1;
            bool output = false, optional = false;
        };
        std::vector<Param> params;
        for (auto& v : Var::parse(group(source, match, 3), match.position(3))) params.emplace_back(std::move(v));

        // Find dispatchable handle.
        Handle* handle = findHandle(params[0]);
        if (handle) {
            if (params[0].constant || params[0].pointers)
                throw std::runtime_error("Unexpected handle parameter: " + std::string(params[0].type));
            params.erase(params.begin());
        }

        // Convert name.
        Token originalName = group(source, match, 2);       // E.g. vmaCreateAllocator
        Token name = originalName.substr(3).capitalize(false); // E.g. createAllocator
        if (handle && startsWith(*name, "destroy") && (*name).substr(7) == *handle->name)
            name = "destroy"; // E.g. Allocator::destroyAllocator -> Allocator::destroy

        if (!handle) functions.emplace_back(name, match);

        // Find dependencies of array sizes.
        for (int i = 0; i < params.size(); ++i) {
            // Ignore void* - those should not be represented as arrays.
            if (auto& p = params[i]; p.lenIfNotNull && (p.pointers > 1 || p.kind != Var::Kind::VOID)) {
                for (int j = 0; j < params.size(); ++j) {
                    if (*p.lenIfNotNull == *params[j].name) {
                        p.deducedArraySize = params[j].deducedArraySize = j;
                        break;
                    }
                }
            }
        }

        // Make parameter names pretty.
        for (auto& v : params) {
            if (v.pointers && (*v.name)[0] == 'p')
                v.name = v.name.substr(v.pointers == 2 && (*v.name)[1] == 'p' ? 2 : 1).capitalize(false);
        }

        // Find output params.
        std::vector<Param*> outputs; // Mandatory only.
        int defaultOutputsFrom = 0;
        for (int i = 0; i < params.size(); ++i) {
            // Ignore void* and char* - those are not outputs.
            if (auto& p = params[i]; !p.constant && p.pointers && (p.pointers > 1 || (p.kind != Var::Kind::VOID && p.kind != Var::Kind::CHAR))) {
                if (p.tag == Var::Tag::NOT_NULL) {
                    outputs.push_back(&p);
                    p.output = true;
                } else {
                    p.output = p.optional = true;
                    if (defaultOutputsFrom > i) defaultOutputsFrom = i;
                    continue;
                }
            }
            defaultOutputsFrom = params.size();
        }

        // Convert return type.
        auto originalReturnType = group(source, match, 1);
        Token simpleReturn, enhancedReturn, uniqueReturn, enhancedVecAllocReturn, uniqueVecAllocReturn, vecAllocName;
        if (outputs.empty() && originalReturnType == "VkBool32") simpleReturn = enhancedReturn = "VULKAN_HPP_NAMESPACE::Bool32";
        else if (originalReturnType == "VkResult") simpleReturn = "VULKAN_HPP_NAMESPACE::Result";
        else if (originalReturnType == "void") simpleReturn = "void";
        if (!outputs.empty()) {
            // bool hasUniqueVariant = false, hasArray = false;
            // std::array<std::string, outputs.size()> uniqueOutputs;
            // for (int i = 0; i < outputCount; ++i) {
            //     auto& s = uniqueOutputs[i];
            //     const auto& p = params[outputs[i]];
            //     // if (p.kind == Var::Kind::VK) {
            //     //     if ()
            //     // }
            //     if (!s.empty()) hasUniqueVariant = true;
            //     if (p.lenIfNotNull) hasArray = true;
            // }
            //
            //
            //
            // if (outputCount == 1 && !hasArray) {
            //     if (params[outputs[0]].pointers == 1) enhancedReturn = "a";
            //     if (params[outputs[0]].pointers == 2) {
            //         enhancedReturn = "a";
            //     }
            // } else if (outputCount == 2 && !hasArray) {
            //     if (params[outputs[0]].pointers == 1 && params[outputs[1]].pointers == 1) enhancedReturn = "a";
            // } else if (outputCount == 1 && hasArray) {
            //     if (params[outputs[0]].pointers == 1) enhancedReturn = "a";
            // }
        } else if (!enhancedReturn) enhancedReturn = "void";
        if (!enhancedReturn) throw std::runtime_error("Unexpected output configuration for " + std::string(originalName));

        // Generate methods.
        Segment::Vector<10> all;
        auto& [
            // Simple variant (returning error code).
            simpleDecl, simpleDef,
            // Enhanced variant (augmented parameters and result code checks) + UniqueHandles.
            enhancedDecl, enhancedDef, uniqueDecl, uniqueDef,
            // Enhanced variants with custom vector allocator.
            enhancedVecAllocDecl, enhancedVecAllocDef, uniqueVecAllocDecl, uniqueVecAllocDef
        ] = *all;

        // Generate template for vector allocator.
        // if (outputCount == 1 && params[outputs[0]].lenIfNotNull) {
            // if (!customVectorAllocator) decl = generate(definition, uniqueHandle, true) + "\n";
            // decl += bindingDecl + "template<typename VectorAllocator";
            // if (!definition) decl += " = std::allocator<" + returnType(0, uniqueHandle) + ">";
            // if (customVectorAllocator) {
            //     decl += ",\n         typename B";
            //     if (!definition) decl += " = VectorAllocator";
            //     decl += ",\n         typename std::enable_if<std::is_same<typename B::value_type, " +
            //             returnType(0, uniqueHandle) + ">::value, int>::type";
            //     if (!definition) decl += " = 0";
            // }
            // decl += ">\n";
        // }

        continue;
    }

    return { handles, functions };
}

void generateModule(const ConditionalTree& tree, const Symbols& enums, const Symbols& structs, const Symbols& handles, const Symbols& functions) {
    Segment exports, specializations;
    for (const Symbol& t : enums) {
        exports + n + t.navigate + "using VMA_HPP_NAMESPACE::" + t.name + ";";
        if (endsWith(*t.name, "FlagBits"))
            specializations + n + t.navigate + "template<> struct FlagTraits<VMA_HPP_NAMESPACE::" + t.name + ">;";
    }
    for (const Symbol& t : structs)
        exports + n + t.navigate + "using VMA_HPP_NAMESPACE::" + t.name + ";";
    (exports, specializations) + navigate.reset;

    R"(// Generated from the Vulkan Memory Allocator (vk_mem_alloc.h).
    module;
    #define VMA_IMPLEMENTATION
    #include <vk_mem_alloc.hpp>
    //#include <vk_mem_alloc_raii.hpp> TODO
    export module vk_mem_alloc_hpp;

    export namespace VMA_HPP_NAMESPACE {
    #ifndef VULKAN_HPP_NO_TO_STRING
      using VMA_HPP_NAMESPACE::to_string;
    #endif
      using VMA_HPP_NAMESPACE::functionsFromDispatcher;
      using VMA_HPP_NAMESPACE::operator|;
      using VMA_HPP_NAMESPACE::operator&;
      using VMA_HPP_NAMESPACE::operator^;
      using VMA_HPP_NAMESPACE::operator~;
      $0
    }

    module : private;
    namespace VULKAN_HPP_NAMESPACE {
      // This is needed for template specializations to be visible outside the module when importing vulkan_hpp (is this a MSVC bug?).
      $1
    }
    )"_seg.replace(exports, specializations).resolve(tree).generate("vk_mem_alloc.cppm");
}

std::string readSource() {
    std::ifstream in(BASE_PATH "/VulkanMemoryAllocator/include/vk_mem_alloc.h");
    std::string text;
    in.seekg(0, std::ios::end);
    text.reserve(in.tellg());
    in.seekg(0, std::ios::beg);
    text.assign(std::istreambuf_iterator(in), {});

    text = std::regex_replace(text, std::regex(R"(/\*[\s\S]*?\*/)"), ""); // Delete multi-line comments
    text = std::regex_replace(text, std::regex("//.*"), ""); // Delete single-line comments
    text = text.substr(0, text.find("#ifdef VMA_IMPLEMENTATION")); // Strip implementation part
    return text;
}

int main(int, char**) {
    try {
        const auto sourceString = readSource();
        const Source source { sourceString };

        const auto enums = generateEnums(source);
        const auto structs = generateStructs(source);
        const auto [handles, functions] = generateHandles(source);
        generateModule(source.tree, enums, structs, handles, functions);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}