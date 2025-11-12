#include <algorithm>
#include <array>
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
    explicit Iterable(Iterator b, Iterator e = {}) : b(std::move(b)), e(std::move(e)) {}
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
std::string docLink(const std::string_view& s) {
    std::string result;
    result.reserve(s.length());
    for (int i = 0; i < s.length(); ++i) {
        const char c = capitalize(s[i], false);
        if (i > 0 && c != s[i]) result += '_';
        result += c;
    }
    return result;
}
std::string camelCase(const std::string_view& s) {
    std::string result;
    result.reserve(s.length());
    for (int i = 0, u = 1; i < s.length(); ++i) {
        if (const char c = s[i]; c != '_') {
            result += capitalize(c, u);
            u = 0;
        } else u = 1;
    }
    return result;
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
    struct ConditionalNode {
        using Data = std::tuple<Segment, int /* mask */, int /* condition */>;
        std::shared_ptr<Data> data;
    };
    struct StringViewNode {
        const char* data;
        int length, indent;
    };
    struct StringNode {
        using Data = std::pair<std::string, int /* indent */>;
        std::shared_ptr<Data> data;
    };
    using Node = std::variant<BreakNode, NavigateNode, WildcardNode, ConditionalNode, StringViewNode, StringNode>;

    struct NodeInfo {
        std::optional<std::string_view> view;
        int indent;
    };
    static std::optional<NodeInfo> info(const Node& node) {
        if (const auto* n = std::get_if<StringViewNode>(&node))
            return {{ std::string_view(n->data, static_cast<unsigned int>(n->length)), n->indent }};
        if (const auto* n = std::get_if<StringNode>(&node))
            return {{ n->data->first, n->data->second }};
        if (const auto* n = std::get_if<WildcardNode>(&node))
            return {{ std::nullopt, n->indent }};
        return std::nullopt;
    }

    template<class T, std::size_t> using ArrayTupleElement = T;
    template<class T, std::size_t... I>
    static auto ArrayTupleType(std::index_sequence<I...>) -> std::tuple<ArrayTupleElement<T, I>...>* { return nullptr; }
    template<class T, std::size_t S> using ArrayTuple = std::remove_pointer_t<decltype(ArrayTupleType<T>(std::make_index_sequence<S>()))>;
    template <class Expected, class T> using ForwardReturn = std::enable_if_t<std::is_same_v<Expected, std::decay_t<T>>, T>;

public:
    class Specialization {
        friend class Segment;
        int mask, condition;
        explicit constexpr Specialization(const int mask, const int condition) : mask(mask), condition(condition) {}
    public:
        explicit constexpr Specialization(const int mask) : mask(mask), condition(mask) {}
        explicit constexpr operator bool() const { return !(~mask & condition); }
        Specialization constexpr operator!() const { return Specialization(mask, mask ^ condition); }
        bool constexpr operator==(const Specialization& s) const { return mask == s.mask && condition == s.condition; }
        bool constexpr operator!=(const Specialization& s) const { return !(*this == s); }
        Specialization constexpr operator&(const Specialization& s) const {
            return Specialization((s.mask|mask)^s.mask&condition^mask&s.condition, s.condition|condition);
        }
        template <class T> ConditionalNode operator[](T&& t) const {
            return ConditionalNode { std::make_shared<ConditionalNode::Data>(std::forward<T>(t), mask, condition) };
        }
    };

    template<int Size, class T = Segment> class Vector : public ArrayTuple<T, Size> {
        template<bool Back, class Arg, std::size_t I = 0> void concat(const Arg& arg) {
            if constexpr (Back) std::get<I>(*this) << arg;
            else arg >>= std::get<I>(*this);
            if constexpr (I < Size - 1) concat<Back, Arg, I + 1>(arg);
        }
        template<std::size_t... I> Vector<Size + 1, T> join(T&& o, std::index_sequence<I...>) && {
            return { std::forward<T>(std::get<I>(*this))..., std::forward<T>(o) };
        }
    public:
        using ArrayTuple<T, Size>::ArrayTuple;
        auto& operator*() & { return static_cast<ArrayTuple<T, Size>&>(*this); }
        Vector<Size + 1, T> operator,(T&& o) && { return std::move(*this).join(std::forward<T>(o), std::make_index_sequence<Size>()); }
        template<std::size_t I = 0> Vector& pop() {
            std::get<I>(*this).pop();
            if constexpr (I < Size - 1) pop<I + 1>();
            return *this;
        }
        template<class V, class Arg> friend ForwardReturn<Vector, V> operator<<(V&& v, const Arg& arg) {
            v.template concat<true>(arg);
            return std::forward<V>(v);
        }
        template<class V, class Arg> friend ForwardReturn<Vector, V> operator>>=(const Arg& arg, V&& v) {
            v.template concat<false>(arg);
            return std::forward<V>(v);
        }
    };

    class Token : public Node {
        using Node::Node;
    public:
        Token() : Node(StringViewNode { nullptr, 0, 0 }) {}
        Token(const char* s, const int indent = 0) : Node(StringViewNode { s, static_cast<int>(std::strlen(s)), indent }) {}
        template<std::size_t N> Token(const char(&s)[N], const int indent = 0) : Node(StringViewNode { s, N - 1, indent }) {}
        Token(const std::string_view& s, const int indent = 0) : Node(StringViewNode { s.data(), static_cast<int>(s.length()), indent }) {}
        Token(const std::string& s, const int indent = 0) : Node(StringNode { std::make_shared<StringNode::Data>(StringNode::Data { s, indent }) }) {}
        explicit operator std::string() const { return std::string(**this); }
        std::size_t length() const { return (**this).length(); }
        explicit operator bool() const { return length() > 0; }
        friend bool operator==(const Token& a, const Token& b) { return *a == *b; }
        friend bool operator!=(const Token& a, const Token& b) { return *a != *b; }
        std::string_view operator*() const { return *info(*this)->view; }
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
        if (indent == 0) return;
        if (auto* n = std::get_if<WildcardNode>(&t)) n->indent += indent;
        else if (auto* n = std::get_if<StringViewNode>(&t)) n->indent += indent;
        else if (auto* n = std::get_if<StringNode>(&t)) n->data = std::make_shared<StringNode::Data>(
            StringNode::Data { n->data->first, n->data->second + indent });
        else if (auto* n = std::get_if<ConditionalNode>(&t))
            for (Node& i : std::get<0>(*(n->data = std::make_shared<ConditionalNode::Data>(*n->data))).nodes)
                indentNode(i, indent);
    }
    static void replaceNode(int& i, const int, const std::nullopt_t&) { i++; }
    void replaceNode(const int i, const int indent, const Token& replacement) {
        indentNode(nodes[i] = replacement, indent);
    }
    void replaceNode(const int i, const int indent, const Segment& replacement) {
        if (!replacement.nodes.empty()) {
            nodes[i] = replacement.nodes[0];
            if (replacement.nodes.size() > 1)
                nodes.insert(nodes.begin() + i + 1, replacement.nodes.begin() + 1, replacement.nodes.end());
            if (indent != 0) for (int j = 0; j < replacement.nodes.size(); ++j) indentNode(nodes[i + j], indent);
        } else nodes.erase(nodes.begin() + i);
    }

    template<class... Args>
    void replaceImpl(const std::optional<Specialization> specialization, const Args&... args) {
        using Replacer = void (*)(Segment&, int&, int, const void*);
        const void* replacement[] { &args... };
        Replacer replacers[] { ([](Segment& segment, int& i, int indent, const void* ptr) {
            segment.replaceNode(i, indent, *static_cast<const Args *>(ptr));
            i--;
        })... };
        for (int i = 0, indent = 0; i < nodes.size(); ++i) {
            if (const auto* n = std::get_if<WildcardNode>(&nodes[i]); n && n->index < sizeof...(Args))
                replacers[n->index](*this, i, std::max(n->indent, indent), replacement[n->index]);
            else if (const auto* n = std::get_if<ConditionalNode>(&nodes[i]); n && specialization)
                replaceNode(i--, 0, *specialization & Specialization(std::get<1>(*n->data), std::get<2>(*n->data)) ? std::get<0>(*n->data) : Segment());
            else if (std::holds_alternative<BreakNode>(nodes[i])) indent = 0;
            else if (const auto t = info(nodes[i])) {
                if (indent < t->indent) indent = t->indent;
                if (t->view) indent += static_cast<int>(t->view->length());
            }
        }
    }

public:
    using Break = BreakNode;
    using Navigate = NavigateNode;

    Segment() = default;
    Segment(const Token& t) { nodes.emplace_back(t); }

    template<std::size_t N> explicit Segment(const char(&source)[N]) : Segment(source, N - 1) {}
    friend Segment operator ""_seg(const char* data, const std::size_t size) { return Segment(data, size); }

    Segment& clear() & { nodes.clear(); return *this; }
    Segment& pop() & { if (!nodes.empty()) nodes.pop_back(); return *this; }

    /**
     * Has any text, conditional or wildcard tokens (navigation and breaks are ignored).
     */
    explicit operator bool() const {
        for (const auto& n : nodes)
            if (const auto i = info(n); (i && (!i->view || !i->view->empty())) ||
                std::holds_alternative<ConditionalNode>(n)) return true;
        return false;
    }

    Vector<2, Segment&> operator,(Segment& o) & { return { *this, o }; }

    template<class S> friend ForwardReturn<Segment, S> operator<<(S&& s, const Token& t) {
        s.nodes.emplace_back(t);
        return std::forward<S>(s);
    }
    template<class S> friend ForwardReturn<Segment, S> operator>>=(const Token& t, S&& s) {
        s.nodes.emplace(s.nodes.begin(), t);
        return std::forward<S>(s);
    }
    template<class S> friend ForwardReturn<Segment, S> operator<<(S&& s, const Segment& o) {
        s.nodes.insert(s.nodes.end(), o.nodes.begin(), o.nodes.end());
        return std::forward<S>(s);
    }
    template<class S> friend ForwardReturn<Segment, S> operator>>=(const Segment& o, S&& s) {
        s.nodes.insert(s.nodes.begin(), o.nodes.begin(), o.nodes.end());
        return std::forward<S>(s);
    }

    template<class... Args> Segment replace(const Args&... args) && { replaceImpl(std::nullopt, args...); return std::move(*this); }
    template<class... Args> Segment replace(Specialization spec, const Args&... args) && { replaceImpl(spec, args...); return std::move(*this); }

    Segment resolve(const ConditionalTree& tree, int startPosition = 0) &&;

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
                const auto i = info(node);
                if (!i || !i->view) throw std::runtime_error("Unexpanded node");
                if (i->indent > length) {
                    out << std::setw(i->indent - length) << "";
                    length += i->indent - length;
                }
                length += i->view->length();
                out << *i->view;
            }
        }
        // Trailing breaks are ignored.
    }
};
std::unordered_map<const char*, const Segment> Segment::cache;

using Token = Segment::Token;
constexpr Segment::Break n { 1 }, nn { 2 };
struct {
    template<class T> static int sourcePosition(const T& t) { return static_cast<int>(t.sourcePosition); }
    static int sourcePosition(const long long sourcePosition) { return static_cast<int>(sourcePosition); }
    static int sourcePosition(const long sourcePosition) { return static_cast<int>(sourcePosition); }
    static int sourcePosition(const int sourcePosition) { return sourcePosition; }
    static int sourcePosition(const Match& match) { return static_cast<int>(match.position()); }
    const Segment::Navigate reset { -1 };
    template<class... T> Segment::Navigate operator()(const T&... t) const { return { (sourcePosition(t) + ...) }; }
} constexpr navigate;

/**
* Tree of conditional preprocessing (#if - #elif - #else - #endif) blocks.
*/
class ConditionalTree {
    struct Statement {
        Token directive;
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
            "ifndef _VMA_FUNCTION_HEADERS",
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
                if (to > from) ascent << n << "#" << at(next).directive;
            // Find target statement.
            int target;
            for (int from = statement, to = current().ascent; (target = (from + to) / 2) > from;)
                (sourcePosition < at(target).sourcePosition ? to : from) = target;
            while (at(target).type == Statement::Type::ENDIF) target = at(target).ascent;
            // Descend into the target (ascend from the target, actually).
            for (int i = target; i != statement; i = at(i).descent)
                n >>= "#" >>= at(i).directive >>= descent;
            statement = target;
            return ascent || descent ? ascent << descent << n : ascent;
        }
    };
    Traversal traverse(const int start = 0) const { return Traversal(this, start); }
};

Segment Segment::resolve(const ConditionalTree& tree, const int startPosition) && {
    auto traverse = tree.traverse(startPosition);
    for (int i = 0; i < nodes.size(); ++i)
        if (const auto* n = std::get_if<NavigateNode>(&nodes[i]))
            replaceNode(i--, std::numeric_limits<int>::min(), traverse.navigate(n->sourcePosition));
    return std::move(*this);
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
    int sourcePosition;
    Symbol(const Symbol&) = default;
    template<class... T> explicit Symbol(Token name, const T&... t) : name(std::move(name)), sourcePosition((navigate.sourcePosition(t) + ...)) {}
};

/**
 * Parsed variable declaration (field or function parameter).
 */
class Var {
    Segment typeTemplate;
public:
    Token name; // Original name
    Token prettyName; // Name with p* prefix removed for pointers
    Token underlyingType; // Original type without array size, const and pointers
    Token typeNamespace; // Namespace of the converted C++ type
    Token type; // Converted C++ type name
    Segment simpleType; // Full converted C++ type
    Segment prettyType; // Full converted C++ type with the last pointer stripped
    Token lenIfNotNull;
    enum class Kind : std::uint8_t { OTHER, VOID, CHAR, PFN, VK, VMA, ARRAY } kind;
    enum class Tag : std::uint8_t { NONE, NULLABLE, NOT_NULL } tag;
    std::uint8_t pointers;
    bool constant;
    int sourcePosition;
    int deducedArraySize = -1; // Array params point to the corresponding size param (if found), or -1
    std::vector<int> deducedArrays; // Size param points to corresponding arrays.

    template<class Nm, class Ns, class Arr = Token>
    Segment makeType(const Nm& nm, const Ns& nspace, const int ptrs, const Arr& arrayType = "std::array") const {
        return Segment(typeTemplate).replace(nm, nspace, ptrs == 2 ? "**" : ptrs == 1 ? "*" : "", arrayType);
    }
    template<class Nm, class Ns> Segment makeType(const Nm& nm, const Ns& nspace) const { return makeType(nm, nspace, pointers); }
    template<class Nm> Segment makeType(const Nm& nm) const { return makeType(nm, typeNamespace); }
    template<class Nm, class Ns> Segment makePretty(const Nm& nm, const Ns& nspace) const { return makeType(nm, nspace, pointers - 1); }
    template<class Nm> Segment makePretty(const Nm& nm) const { return makePretty(nm, typeNamespace); }

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

        type = underlyingType = group(2);
        if (type == "uint32_t" || type == "size_t" || type == "float" || type == "HANDLE") kind = Kind::OTHER;
        else if (type == "void") kind = Kind::VOID;
        else if (type == "char") kind = Kind::CHAR;
        else if (startsWith(*type, "PFN_")) kind = Kind::PFN;
        else if (startsWith(*type, "Vk")) {
            kind = Kind::VK;
            typeNamespace = "VULKAN_HPP_NAMESPACE::";
            type = type.substr(2);
        } else if (startsWith(*type, "Vma")) {
            kind = Kind::VMA;
            type = type.substr(3);
        } else throw std::runtime_error("Unknown type: " + std::string(type));

        typeTemplate = "$1$0$2"_seg;
        constant = !group(1).empty();
        const bool p1 = !group(3).empty(), p2 = !group(5).empty();
        pointers = p1 + p2;
        if (constant && pointers > 0) "const " >>= typeTemplate;
        name = group(9);
        prettyName = pointers && name.length() > 0 && (*name)[0] == 'p' ? name.substr(1).capitalize(false) : name;

        std::string_view arr = trim(group(10));
        while (!(arr = trim(arr)).empty()) {
            const auto i = arr.find_last_of('['); // assert(i != npos);
            std::string dim { trim(arr.substr(i + 1, arr.length() - i - 2)) };
            if (startsWith(dim, "VK_")) dim = "VULKAN_HPP_NAMESPACE::" + camelCase(dim.substr(3));
            "$3<"_seg >>= typeTemplate << ", " << dim << ">";
            arr = arr.substr(0, i);
            kind = Kind::ARRAY;
        }
        simpleType = makeType(type, typeNamespace);
        prettyType = makePretty(type, typeNamespace);

        const std::string_view tag = p2 ? group(6) : group(4);
        if (tag == "NULLABLE") this->tag = Tag::NULLABLE;
        else if (tag == "NOT_NULL") this->tag = Tag::NOT_NULL;
        else this->tag = Tag::NONE;

        if (group(7) == "VMA_LEN_IF_NOT_NULL") lenIfNotNull = group(8);
    }

public:

    template<class T = Var>
    static std::vector<T> parse(const std::string_view& source, const long long int sourcePositionOffset = 0) {
        static std::regex pattern { Var::pattern.data(), Var::pattern.length() };
        std::vector<T> result;
        for (const auto& match : iterate(source, pattern))
            result.push_back(T(source, match, static_cast<int>(match.position() + sourcePositionOffset)));
        // Find dependencies of array sizes.
        for (int i = 0; i < result.size(); ++i) {
            // Ignore void* - those should not be represented as arrays.
            if (auto& a = result[i]; a.lenIfNotNull && (a.pointers > 1 || a.kind != Kind::VOID)) {
                for (int j = 0; j < result.size(); ++j) {
                    if (auto& s = result[j]; a.lenIfNotNull == s.name) {
                        s.deducedArrays.push_back(i);
                        a.deducedArraySize = j;
                        break;
                    }
                }
            }
        }
        return result;
    }
};

struct Symbols {
    std::vector<Symbol> enums, structs;
    struct : std::vector<Symbol> {
        std::vector<Symbol> unique, raii;
        const std::vector<Symbol>* operator&() const { return this; }
    } handles, functions;
};

void generateEnums(const Source& source, Symbols& symbols) {
    const std::regex typedefPattern { R"(typedef\s+enum\s+Vma(\w+))" };
    const std::regex entryPattern { R"((VMA_\w+)[^,}]*)" };

    Segment::Vector<3> enumPieces;
    auto& [content, toString, flagTraits] = *enumPieces;
    for (const auto& match : iterate(source, typedefPattern)) {
        Token name = group(source, match, 1); // E.g. AllocationCreateFlagBits
        Token flagBits;                       // E.g. AllocationCreateFlags
        if (endsWith(*name, "FlagBits")) flagBits = std::string(name.substr(0, name.length() - 4)) + "s";
        Token baseName = flagBits ? name.substr(0, name.length() - 8) : name; // E.g. AllocationCreate
        symbols.enums.emplace_back(name, match);
        if (flagBits) symbols.enums.emplace_back(flagBits, match);

        Segment::Vector<4> entryPieces;
        auto& [entries, entryToString, allFlags, flagsToString] = *entryPieces;

        const auto begin = match.position() + match.length();
        const auto body = source.substr(begin, source.find("}", begin) - begin);
        for (const auto& entryMatch : iterate(body, entryPattern)) {
            auto originalEntry = group(body, entryMatch, 1);
            if (endsWith(originalEntry, "_MAX_ENUM")) break;
            bool bitEntry = endsWith(originalEntry, "_BIT"); // assert(!bitEntry || flagBits)

            Token entry = [&] {
                std::string e = camelCase(originalEntry.substr(4/*VMA_*/, originalEntry.length() - 4 - (bitEntry ? 4/*_BIT*/ : 0)));
                // Strip base enum name.
                if (!startsWith(e, *baseName)) throw std::runtime_error("Unexpected enum value: " + std::string(e));
                e.erase(e.begin(), e.begin() + static_cast<int>(baseName.length()));
                return e;
            }();

            entryPieces << navigate(begin, entryMatch);
            entries << n << "e" << entry << " = " << originalEntry << ",";
            entryToString << n << "if (value == " << name << "::e" << entry << ") return \"" << entry << "\";";
            if (flagBits && bitEntry) {
                allFlags << n << (allFlags ? "|" : " ") << " VMA_HPP_NAMESPACE::" << name << "::e" << entry;
                flagsToString << n << "if (value & " << name << "::e" << entry << ") result += \" " << entry << " |\";";
            }
        }
        entries.pop();
        entryPieces << navigate(match);

        (content, toString) << nn << navigate(match);

        content << R"(
        // wrapper class for enum Vma$0, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_enum.html
        enum class $0$1 {
          $2
        };
        )"_seg.replace(name, flagBits ? " : Vma"_seg << flagBits : ""_seg, entries);
        if (flagBits) content << "// wrapper using for bitmask Vma" << flagBits <<
            ", see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_enum.html" <<
            n << "using " << flagBits << " = VULKAN_HPP_NAMESPACE::Flags<" << name << ">;";

        toString << R""(
        VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string($0 value) {
          $1
          return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
        }
        )""_seg.replace(name, entryToString);

        if (flagBits) {
            flagTraits << nn << navigate(match) << R"(

            template<> struct FlagTraits<VMA_HPP_NAMESPACE::$0> {
              using WrappedType = Vma$0;
              static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
              static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<VMA_HPP_NAMESPACE::$0> allFlags =
                $1;
            };
            )"_seg.replace(name, allFlags);

            toString << R"(

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
        entryPieces << navigate.reset;
    }

    R"(
    namespace VMA_HPP_NAMESPACE {
      $0
    }

    namespace VULKAN_HPP_NAMESPACE {
      $1
    }
    )"_seg.replace(content, flagTraits).resolve(source.tree).generateHpp("enums");
    R"(
    namespace VMA_HPP_NAMESPACE {
      $0
    }
    )"_seg.replace(toString).resolve(source.tree).generateHpp("to_string");
}

void generateStructs(const Source& source, Symbols& symbols) {
    const std::regex typedefPattern { R"(typedef\s+struct\s+Vma(\w+))" };

    Segment content;
    for (const auto& match : iterate(source, typedefPattern)) {
        Token name = group(source, match, 1); // E.g. DeviceMemoryCallbacks
        symbols.structs.emplace_back(name, match);

        const auto begin = match.position() + match.length();
        const auto body = source.substr(begin, source.find("}", begin) - begin);
        std::vector<Var> members = Var::parse(body, begin);

        Token constexprToken = "VULKAN_HPP_CONSTEXPR";
        Segment::Vector<9> memberPieces;
        auto& [constructorParams, constructorInitializer, enhancedParams, enhancedInitializer,
               setters, reflectTypes, reflectTie, comparison, fields] = *memberPieces;
        bool containsPFN = false, hasEnhancedConstructor = false;
        for (const Var& member : members) {
            Segment storageType = member.simpleType;
            if (member.kind == Var::Kind::ARRAY) {
                storageType = member.makeType(member.type, member.typeNamespace, member.pointers, "VULKAN_HPP_NAMESPACE::ArrayWrapper1D");
                constexprToken = "VULKAN_HPP_CONSTEXPR_14";
            } else containsPFN |= member.kind == Var::Kind::PFN;
            memberPieces << navigate(member);

            // Generate constructor parameter and setter.
            if (constructorParams)
                (constructorParams, constructorInitializer, reflectTypes, reflectTie) << n << Token(", ", -2);
            constructorParams << member.simpleType << " " << member.name << "_ = {}";
            constructorInitializer << member.name << " { " << member.name << "_ }";
            setters << R"(
            VULKAN_HPP_CONSTEXPR_14 $0& set$3($2 $1_) VULKAN_HPP_NOEXCEPT {
              $1 = $1_;
              return *this;
            }

            )"_seg.replace(name, member.name, member.simpleType, member.name.capitalize());

            // Generate enhanced constructor parameter and setter.
            if (member.deducedArrays.empty()) { // Skip deduced size params.
                if (enhancedParams) (enhancedParams, enhancedInitializer) << n << Token(", ", -2);
                Token defaultAssignment = hasEnhancedConstructor ? " = {}" : ""; // No default assignments till the first enhanced param (inclusive).
                if (member.deducedArraySize != -1) {
                    hasEnhancedConstructor = true;
                    const auto& size = members[member.deducedArraySize];
                    if (size.deducedArrays.size() > 1) throw std::runtime_error("Deducing size from multiple arrays"); // Not implemented.
                    enhancedParams << "VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<" << member.prettyType << "> const & " << member.prettyName << "_";
                    enhancedInitializer << size.name << " { static_cast<" << size.simpleType << ">(" << member.prettyName << "_.size()) }";
                    enhancedInitializer << n << Token(", ", -2) << member.name << " { " << member.prettyName << "_.data() }";
                    setters << R"(
                    #if !defined( VULKAN_HPP_DISABLE_ENHANCED_MODE )
                    $0& set$1(VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<$5> const & $2_) VULKAN_HPP_NOEXCEPT {
                      $4 = static_cast<$6>($2_.size());
                      $3 = $2_.data();
                      return *this;
                    }
                    #endif

                    )"_seg.replace(name, member.prettyName.capitalize(), member.prettyName, member.name, size.name, member.prettyType, size.simpleType);
                } else {
                    enhancedParams << member.simpleType << " " << member.name << "_";
                    enhancedInitializer << member.name << " { " << member.name << "_ }";
                }
                enhancedParams << defaultAssignment;
            }

            // Generate comparison and field declarations.
            reflectTypes << storageType << " const &";
            reflectTie << member.name;
            if (comparison) comparison << n << Token("&& ", -3);
            comparison << member.name << " == rhs." << member.name;
            fields << n << storageType << " " << member.name << " = {};";
        }
        setters.pop();
        memberPieces << navigate(match);

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

        Segment constructors = R"(
        $1 $0(
            $2
        ) VULKAN_HPP_NOEXCEPT
          : $3 {}

        $1 $0($0 const &) VULKAN_HPP_NOEXCEPT = default;
        $0(Vma$0 const & rhs) VULKAN_HPP_NOEXCEPT : $0(*reinterpret_cast<$0 const *>(&rhs)) {}
        )"_seg.replace(name, constexprToken, constructorParams, constructorInitializer);
        if (hasEnhancedConstructor) {
            constructors << R"(

            #if !defined( VULKAN_HPP_DISABLE_ENHANCED_MODE )
            $0(
                $1
            ) VULKAN_HPP_NOEXCEPT
              : $2 {}
            #endif
            )"_seg.replace(name, enhancedParams, enhancedInitializer);
        }

        content << nn << navigate(match) << R"(
        // wrapper struct for struct Vma$0, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_$1.html
        struct $0 {
          using NativeType = Vma$0;

        #if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
          $2

          $0& operator=($0 const &) VULKAN_HPP_NOEXCEPT = default;
        #endif

          $0& operator=(Vma$0 const & rhs) VULKAN_HPP_NOEXCEPT {
            *this = *reinterpret_cast<VMA_HPP_NAMESPACE::$0 const *>(&rhs);
            return *this;
          }

        #if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
          $3
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
          std::tuple<$4>
          reflect() const VULKAN_HPP_NOEXCEPT {
            return std::tie($5);
          }
        #endif

        $6

        public:
          $7
        };
        )"_seg.replace(name, docLink(*name), constructors, setters, reflectTypes, reflectTie, comparison, fields);

        // Generate functionsFromDispatcher.
        if (name == "VulkanFunctions") {
            Segment pfns, specs, dynamicInit, staticInit;
            for (const Var& member : members) {
                Token pfn = member.kind == Var::Kind::PFN ? member.type.substr(4) : "";
                (pfns, specs, dynamicInit, staticInit) << n << navigate(member);
                (dynamicInit, staticInit) << (staticInit ? ", " : "  ");
                if (pfn) {
                    pfns << R"(
                    struct $0 { using type = PFN_$0; };
                    )"_seg.replace(pfn);
                    specs << R"(
                    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::$0, decltype(T::$0), T, Ts...>
                    { static PFN_$0 get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.$0; } };
                    )"_seg.replace(pfn);
                    dynamicInit << "detail::VulkanPFN::get<detail::VulkanPFN::" << pfn << ">(dispatch...)";
                    staticInit << "&" << pfn;
                } else (dynamicInit, staticInit) << "nullptr";
            }
            (pfns, specs, dynamicInit, staticInit) << navigate.reset;
            content << nn << R"(
            namespace detail {
              struct VulkanPFN {
                $0
                template<class Fun, class PFN, class T, class... Ts> struct FromDispatchers {
                  static PFN get(const T&, const Ts&... ts) VULKAN_HPP_NOEXCEPT {
                    return FromDispatchers<Fun, PFN, Ts...>::get(ts...);
                  }
                };
                template<class Fun, class... Ts> static typename Fun::type get(const Ts&... ts) VULKAN_HPP_NOEXCEPT {
                  return FromDispatchers<Fun, typename Fun::type, Ts...>::get(ts...);
                }
              };
              $1
            }

            // utility function for retrieving function table from vk dispatchers
            template <class... Dispatch> VulkanFunctions
            functionsFromDispatchers(Dispatch const &... dispatch) VULKAN_HPP_NOEXCEPT {
              return VulkanFunctions {
                $2
              };
            }
            template <class Dispatch = VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> VulkanFunctions
            functionsFromDispatcher(Dispatch const & dispatch VULKAN_HPP_DEFAULT_DISPATCHER_ASSIGNMENT) VULKAN_HPP_NOEXCEPT {
              return functionsFromDispatchers(dispatch);
            }
            #if !defined( VK_NO_PROTOTYPES )
            template <> VULKAN_HPP_CONSTEXPR_INLINE VulkanFunctions
            functionsFromDispatcher<VULKAN_HPP_DISPATCH_LOADER_STATIC_TYPE>(VULKAN_HPP_DISPATCH_LOADER_STATIC_TYPE const &) VULKAN_HPP_NOEXCEPT {
              return VulkanFunctions {
                $3
              };
            }
            #endif
            )"_seg.replace(pfns, specs, dynamicInit, staticInit);
        }
    }

    R"(
    namespace VMA_HPP_NAMESPACE {
      $0
    }
    )"_seg.replace(content << navigate.reset).resolve(source.tree).generateHpp("structs");
}

void generateHandles(const Source& source, Symbols& symbols) {
    // All handles.
    struct Handle : Symbol {
        using Symbol::Symbol;
        Segment methodDecl, methodDef, raiiDecl, raiiDef, raiiConstructorDecl, raiiConstructorDef;
        Handle* owner = nullptr;
        Token memberName, destructor;
        bool raiiOnly = true;
    } namespaceHandle {"", -1};
    std::vector<Handle> handleVector;
    handleVector.reserve(16); // We use pointers, so preallocate.

    // Find all handles.
    const std::regex handlePattern { R"(VK_DEFINE_(NON_DISPATCHABLE_)?HANDLE\s*\(\s*Vma(\w+)\s*\))" };
    for (const auto& match : iterate(source, handlePattern)) {
        auto& h = handleVector.emplace_back(group(source, match, 2), match);
        h.memberName = h.name.capitalize(false);
        h.raiiOnly = false;
    }
    handleVector.emplace_back("Buffer", -1).destructor = "destroyBuffer";
    handleVector.emplace_back("Image", -1).destructor = "destroyImage";
    auto& statsStringHandle = handleVector.emplace_back("StatsString", -1);

    // All functions.
    struct Param : Var {
        using Var::Var;
        enum class Type {
            INPUT, OUTPUT, OPTIONAL_OUTPUT
        } paramType = Type::INPUT;
        Handle* handle = nullptr;
    };
    struct Function : Symbol {
        std::string_view returnType;
        std::vector<Param> params;
        Handle* handle = nullptr; // Top-level handle.
        const Param* secondHandleParam = nullptr;
        Token methodName, shortName, raiiName;
        explicit Function(Symbol&& symbol, std::string_view&& returnType, std::vector<Param>&& params) :
            Symbol(symbol), returnType(returnType), params(params) {}
        Iterable<std::vector<Param>::iterator> methodParams() { return Iterable(params.begin() + !!handle, params.end()); }
    };
    std::vector<Function> functionVector;

    // Find all functions.
    const std::regex funcPattern { R"(VMA_CALL_PRE\s+(.+)\s+VMA_CALL_POST\s+(vma\w+)\s*(\([\s\S]+?\)\s*;))" };
    for (const auto& match : iterate(source, funcPattern))
        functionVector.emplace_back(Symbol(group(source, match, 2), match), group(source, match, 1),
                                    Var::parse<Param>(group(source, match, 3), match.position(3)));
    for (auto& function : functionVector) {
        // Map parameters to the handles.
        for (auto& param : function.params) {
            if (param.kind != Var::Kind::VMA && param.kind != Var::Kind::VK) continue;
            for (Handle& handle : handleVector) {
                if (handle.name == param.type) {
                    param.handle = &handle;
                    break;
                }
            }
        }

        // Find the first-order handle.
        if (!function.params.empty() && function.params[0].handle) {
            if (function.params[0].pointers)
                throw std::runtime_error("Unexpected handle parameter in " + std::string(function.name));
            function.handle = function.params[0].handle;

            // Find the second-order handle.
            for (const auto& param : function.methodParams()) {
                if (param.handle == nullptr || param.pointers || param.kind != Var::Kind::VMA) continue;
                // Set the owner for the second-level handle.
                if (!param.handle->owner || param.handle->owner == function.handle) param.handle->owner = function.handle;
                else throw std::runtime_error("Handle owner conflict: " + std::string(param.handle->name));
                function.handle = param.handle;
                function.secondHandleParam = &param;
            }
        }

        // Convert name.
        Token shortName;
        if ((*function.name).find("Destroy") != std::string_view::npos) shortName = "destroy";
        else if ((*function.name).find("Free") != std::string_view::npos) shortName = "free";
        if (shortName && function.handle && !function.handle->owner &&
            function.params.size() == 1) function.methodName = shortName; // First level handle destructors are always short.
        else {
            function.methodName = function.name.substr(3).capitalize(false);
            function.shortName = shortName;
            // Convert RAII name.
            auto replace = [&](const std::initializer_list<std::string_view> aliases) {
                for (const auto& alias : aliases) {
                    if (const auto pos = (*function.name).find(alias); pos != std::string_view::npos) {
                        std::string result;
                        result.reserve(alias.length());
                        result += (*function.name).substr(3, pos - 3);
                        result += (*function.name).substr(pos + alias.length());
                        result[0] = capitalize(result[0], false);
                        return Token(result);
                    }
                }
                return function.methodName;
            };
            std::string_view handle = function.handle ? *function.handle->name : "";
            if (function.name == "vmaFreeStatsString" || function.name == "vmaFreeVirtualBlockStatsString" ||
                function.name == "vmaDestroyBuffer" || function.name == "vmaDestroyImage") function.raiiName = {}; // No RAII variant.
            else if (function.name == "vmaCopyMemoryToAllocation") function.raiiName = "copyFromMemory";
            else if (function.name == "vmaClearVirtualBlock")      function.raiiName = "clearBlock";
            else if (handle == "Allocation")   function.raiiName = replace({"Allocation", "Memory"});
            else if (handle == "VirtualBlock") function.raiiName = replace({"VirtualBlock", "Virtual"});
            else if (function.handle) function.raiiName = replace({ handle });
            else function.raiiName = function.methodName;
        }

        // Save destructor.
        if (shortName && function.handle && function.params.size() == 1 + !!function.handle->owner) {
            if (function.handle->destructor)
                throw std::runtime_error("Conflicting destructor for handle: " + std::string(function.handle->name));
            function.handle->destructor = function.methodName;
            function.raiiName = {}; // Destructors do not have separate methods in RAII.
        }
    }

    // Collect unique & RAII handles.
    for (const auto& h : handleVector) {
        if (!h.raiiOnly) {
            symbols.handles.emplace_back(h);
            if (h.destructor) symbols.handles.unique.emplace_back(h);
        }
        symbols.handles.raii.emplace_back(h);
    }

    // Iterate VMA functions.
    for (auto& function : functionVector) {
        auto& params = function.params;
        auto* handle = function.handle;
        if (handle && handle->owner) handle = handle->owner; // Get the first-level handle.

        // Find output params.
        std::vector<Param*> outputs; // Mandatory only.
        const Param* defaultOutputsFrom = nullptr;
        bool hasUniqueVariant = false, hasRAIIVariant = false;
        for (auto& param : function.methodParams()) {
            // Ignore void* and char* - those are not outputs.
            if (param.pointers > 1 || (!param.constant && param.pointers && param.kind != Var::Kind::VOID && param.kind != Var::Kind::CHAR)) {
                if (param.tag == Var::Tag::NOT_NULL) {
                    param.paramType = Param::Type::OUTPUT;
                    if (Param* o = outputs.emplace_back(&param); o->handle) {
                        hasRAIIVariant = true;
                        // RAII Buffers and Images can only be created from the first level handle.
                        if (o->kind == Var::Kind::VK && function.handle && function.handle->owner) {
                            function.handle = function.handle->owner;
                            function.secondHandleParam = nullptr;
                        }
                    }
                } else {
                    param.paramType = Param::Type::OPTIONAL_OUTPUT;
                    if (defaultOutputsFrom > &param) defaultOutputsFrom = &param;
                    continue;
                }
            }
            defaultOutputsFrom = params.data() + params.size();
        }

        // Find a combined RAII output handle (Buffer or Image).
        // Combined handle is created from at least one already existing component (Buffer, Image, or Allocation).
        Handle* combinedHandle = [&]() -> Handle* {
            bool hasVmaHandle = false, hasInputHandle = false;
            Handle* result = nullptr;
            for (const auto& param : function.methodParams()) {
                if (!param.handle) continue;
                if (param.kind == Var::Kind::VK) result = param.handle;
                else hasVmaHandle = true;
                if (param.paramType == Param::Type::INPUT) hasInputHandle = true;
            }
            return hasVmaHandle && hasInputHandle && function.raiiName ? result : nullptr;
        }();

        // Generate methods.
        struct Method {
            Segment ret, params, body;
        } simple, enhanced, raii;
        Segment::Specialization declaration(1), unique(2), vectorAllocator(4), constructor(8), combining(16);

        // Convert simple return type.
        enum class ResultValue {
            NONE, VALUE, VALUE_TYPE
        } resultValue = ResultValue::NONE;
        Token simpleReturn, vecAllocType, vecAllocName, enhancedOutput = "result";
        Segment enhancedReturn, raiiReturn, enhancedOutputs, uniqueWrap, resultCheck;
        if (function.returnType == "VkBool32") enhancedReturn = raiiReturn = simpleReturn = "VULKAN_HPP_NAMESPACE::Bool32";
        else if (function.returnType == "void") simpleReturn = "void";
        else if (function.returnType == "VkResult") {
            simpleReturn = "VULKAN_HPP_NAMESPACE::Result";
            resultCheck = "VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING \"::$0\""_seg << ");";
            resultValue = ResultValue::VALUE_TYPE;
            // Custom result checks.
            if (function.name == "vmaBeginDefragmentationPass" ||
                function.name == "vmaEndDefragmentationPass") {
                resultValue = ResultValue::VALUE;
                resultCheck.pop() << ", { VULKAN_HPP_NAMESPACE::Result::eSuccess, VULKAN_HPP_NAMESPACE::Result::eIncomplete });";
            } else if (function.name == "vmaFindMemoryTypeIndex" ||
                function.name == "vmaFindMemoryTypeIndexForBufferInfo" ||
                function.name == "vmaFindMemoryTypeIndexForImageInfo")
                "if (result == VULKAN_HPP_NAMESPACE::Result::eErrorFeatureNotPresent) memoryTypeIndex = VULKAN_HPP_NAMESPACE::MaxMemoryTypes;\nelse "_seg >>= resultCheck;
        }

        // Custom RAII type for the stats string.
        if (function.name == "vmaBuildStatsString" || function.name == "vmaBuildVirtualBlockStatsString") {
            statsStringHandle.sourcePosition = function.sourcePosition; // Bind to the constructor function.
            raiiReturn = (outputs[0]->handle = &statsStringHandle)->name;
            hasRAIIVariant = true;
        }

        // Convert enhanced outputs.
        Handle* raiiOutputHandle = nullptr;
        if (!enhancedReturn) {
            Segment enhancedTypes[2];
            bool hasUniqueType[2] {};
            for (int i = 0; i < outputs.size(); ++i) {
                if (const Param& p = *outputs[i]; p.handle && p.handle->destructor) {
                    enhancedTypes[i] << unique[p.makePretty("Unique"_seg << p.type, "")] << (!unique)[p.prettyType];
                    hasUniqueType[i] = true;
                    hasUniqueVariant = true;
                } else enhancedTypes[i] = p.prettyType;
            }
            if (outputs.size() == 2 && !outputs[0]->lenIfNotNull && !outputs[1]->lenIfNotNull) { // Pair.
                enhancedReturn << "std::pair<" << enhancedTypes[1] << ", " << enhancedTypes[0] << ">"; // Pair order is reversed.
                if (outputs[0]->kind == Var::Kind::VK) { // RAII uses combined handles.
                    raiiReturn = outputs[0]->makePretty(outputs[0]->type, "");
                    raiiOutputHandle = outputs[0]->handle;
                }
                enhancedOutput = "pair";
                enhancedOutputs << n << "std::pair<" << outputs[1]->prettyType << ", " << outputs[0]->prettyType << "> pair;" <<
                    n << outputs[0]->prettyType << "& " << outputs[0]->prettyName << " = pair.second;" <<
                    n << outputs[1]->prettyType << "& " << outputs[1]->prettyName << " = pair.first;";
                if (hasUniqueVariant) {
                    uniqueWrap << " {";
                    for (int i = 1; i >= 0; --i) {
                        uniqueWrap << n << "  ";
                        if (hasUniqueType[i]) uniqueWrap << enhancedTypes[i] << "(" << outputs[i]->prettyName << (handle ? ", *this" : ", {}") << ")";
                        else uniqueWrap << outputs[i]->prettyName;
                        uniqueWrap << ",";
                    }
                    uniqueWrap.pop() << n << "};";
                }
            } else if (outputs.size() == 1) {
                enhancedOutput = outputs[0]->prettyName;
                if (!raiiReturn) raiiReturn = outputs[0]->handle ? // Add vk::raii namespace for vk handles.
                    outputs[0]->makePretty(outputs[0]->type, outputs[0]->kind == Var::Kind::VK ?
                        "VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::" : "") : outputs[0]->prettyType;
                if (outputs[0]->lenIfNotNull) { // Vector.
                    vecAllocName = (vecAllocType = outputs[0]->type).capitalize(false);
                    enhancedReturn = "std::vector<$0, $1Allocator>"_seg.replace(enhancedTypes[0], vecAllocType);
                    "std::vector<" >>= raiiReturn << ">";
                    // Generate template for vector allocator.
                    enhanced.ret << "template <typename " << vecAllocType << "Allocator" <<
                        (declaration & !vectorAllocator)[" = std::allocator<$0>"_seg.replace(enhancedTypes[0])] <<
                        vectorAllocator[",\n          typename std::enable_if<std::is_same<typename $1Allocator::value_type, $0>::value, int>::type"_seg.replace(enhancedTypes[0], vecAllocType)] <<
                        (declaration & vectorAllocator)[" = 0"]  << ">" << n;
                    // Unique variants should use a default allocator for the intermediate vector.
                    enhancedOutputs << n << "std::vector<" << outputs[0]->prettyType <<
                        (!unique)[", $0Allocator"_seg.replace(vecAllocType)] << "> " << enhancedOutput;
                    Token size;
                    if (outputs[0]->deducedArraySize == -1) { // Custom size deduction logic.
                        if (function.name == "vmaGetHeapBudgets" && outputs[0]->lenIfNotNull == "\"VkPhysicalDeviceMemoryProperties::memoryHeapCount\"")
                            size = "getMemoryProperties()->memoryHeapCount";
                        else throw std::runtime_error("Could not deduce output vector size for " + std::string(function.name));
                    } else size = params[outputs[0]->deducedArraySize].prettyName;
                    enhancedOutputs << "(" << size << (!unique & vectorAllocator)[", "_seg << vecAllocName << "Allocator"] << ");";
                    if (hasUniqueVariant) {
                        uniqueWrap << vectorAllocator["($0Allocator)"_seg.replace(vecAllocName)] << ";" << n <<
                            enhancedOutput << "Unique.reserve(" << enhancedOutput << ".size());" << n <<
                            "for (auto const& t : " << enhancedOutput << ") " << enhancedOutput << "Unique.emplace_back(t" << (handle ? ", *this" : ", {}") << ");";
                    }
                } else { // Single value.
                    enhancedReturn = std::move(enhancedTypes[0]);
                    raiiOutputHandle = outputs[0]->kind != Var::Kind::VK ? outputs[0]->handle : nullptr;
                    enhancedOutputs << n << outputs[0]->prettyType << " " << enhancedOutput << ";";
                    if (hasUniqueVariant) uniqueWrap << " { " << enhancedOutput << (handle ? ", *this" : ", {}") << " };";
                }
            }
        } else if (!outputs.empty()) simpleReturn = {}; // Trigger error.
        if (uniqueWrap) enhancedReturn >>= " " >>= enhancedOutput >>= "Unique" >>= uniqueWrap;

        // Add function symbols.
        if (!handle) {
            symbols.functions.emplace_back(function.methodName, function);
            if (hasUniqueVariant) symbols.functions.unique.emplace_back(function.methodName, function);
            if (hasRAIIVariant) symbols.functions.raii.emplace_back(function.methodName, function);
        }

        // Verify that conversion succeeded.
        if (!simpleReturn || (!enhancedReturn && !outputs.empty()))
            throw std::runtime_error("Unexpected output configuration for " + std::string(function.name));

        // Generate macros and return type.
        if (resultValue == ResultValue::VALUE || enhancedReturn) (enhanced.ret, raii.ret) << "VULKAN_HPP_NODISCARD ";
        else if (resultValue == ResultValue::VALUE_TYPE) (enhanced.ret, raii.ret) << "VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS ";
        if (simpleReturn != "void") simple.ret << "VULKAN_HPP_NODISCARD ";
        if (resultValue == ResultValue::VALUE) {
            if (combinedHandle)  throw std::runtime_error("Combined handles in ResultValue are not implemented: " + std::string(function.name));
            if (!enhancedReturn) (enhancedReturn, raiiReturn) << simpleReturn;
            else "VULKAN_HPP_NAMESPACE::ResultValue<" >>= (enhancedReturn, raiiReturn) << ">";
        } else {
            if (!enhancedReturn) (enhancedReturn, raiiReturn) << "void";
            if (combinedHandle) { // And combined variant.
                auto t = (!combining)[std::move(raiiReturn)];
                raiiReturn.clear() << std::move(t) << combining[combinedHandle->name];
            }
            if (resultValue == ResultValue::VALUE_TYPE)
                "typename VULKAN_HPP_NAMESPACE::ResultValueType<" >>= (enhancedReturn, raiiReturn) << ">::type";
        }
        (simple.ret, enhanced.ret, raii.ret) << (!declaration)["VULKAN_HPP_INLINE "];
        simple.ret << simpleReturn;
        enhanced.ret << enhancedReturn;
        raii.ret << raiiReturn;

        // Generate parameter list.
        bool signatureTransformed = false;
        for (const auto& param : function.methodParams()) {
            simple.params << param.simpleType << " " << param.name << "," << n;
            if (param.paramType == Param::Type::OUTPUT || !param.deducedArrays.empty()) { // Skip outputs and deduced array sizes.
                signatureTransformed = true;
                continue;
            }
            // Enhanced methods only transform pointer parameters.
            Segment p;
            if (param.pointers && param.kind != Var::Kind::VOID) {
                if (param.lenIfNotNull && !param.constant)
                    p = "VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<$0> const &"_seg;
                else if (param.lenIfNotNull && param.constant)
                    p = "VULKAN_HPP_NAMESPACE::ArrayProxy<$0> const &"_seg;
                else if (param.tag == Var::Tag::NOT_NULL)
                    p = "$0&"_seg;
                else if (!param.constant && param.kind != Var::Kind::OTHER && param.kind != Var::Kind::CHAR)
                    p = "VULKAN_HPP_NAMESPACE::Optional<$0> const &"_seg;
            }
            if (!p) p = param.pointers ? param.simpleType : "$0"_seg;
            else signatureTransformed = true;
            p << " " << param.prettyName;
            if (&param >= defaultOutputsFrom) p << (declaration & !vectorAllocator)[param.pointers ? " = nullptr" : " = {}"];
            if (function.secondHandleParam != &param) {
                // We accept plain handles, so add a non-raii namespace.
                auto r = Segment(p).replace(param.handle && param.kind == Var::Kind::VMA ?
                    param.makePretty("VMA_HPP_NAMESPACE::"_seg << param.type) : param.prettyType);
                if (combinedHandle && param.handle) {
                    raii.params << (!combining)[std::move(r)];
                    r.clear();
                    if (param.kind != Var::Kind::VK) r << param.prettyType;
                    else r << param.makePretty(param.type, "VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::");
                    raii.params << combining[std::move(r << "&& " << param.prettyName)];
                } else raii.params << r;
                raii.params << "," << n;
            }
            enhanced.params << std::move(p).replace(param.prettyType) << "," << n;
        }
        (simple.params, enhanced.params, raii.params).pop().pop();
        enhanced.params << vectorAllocator[(enhanced.params ? ",\n"_seg : ""_seg) <<
            vecAllocType << "Allocator& " << vecAllocName << "Allocator"];

        // Generate deduction statements.
        for (const auto& param : function.methodParams()) {
            if (param.deducedArrays.empty()) continue;
            Segment assertChecks, exceptionChecks;
            enhanced.body << n << param.simpleType << " " << param.prettyName << " = ";
            for (int i : param.deducedArrays) {
                const auto& t = params[i];
                if (t.paramType == Param::Type::OUTPUT) continue;
                enhanced.body << t.prettyName << ".size()" << " | ";
                assertChecks << n << "VULKAN_HPP_ASSERT(";
                exceptionChecks << n << "if (";
                if (t.tag != Var::Tag::NOT_NULL) {
                    assertChecks << t.prettyName << ".empty() || ";
                    exceptionChecks << "!" << t.prettyName << ".empty() && ";
                }
                assertChecks << t.prettyName << ".size() == " << param.prettyName << ");";
                exceptionChecks << t.prettyName << ".size() != " << param.prettyName <<
                    ") throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING \"::$0: "_seg <<
                    t.prettyName << ".size() != " << param.prettyName << "\");";
            }
            enhanced.body.pop() << ";";
            if (param.deducedArrays.size() > 1) // No need to cross-check array sizes when there is only one.
                enhanced.body << n << R"(
                #ifdef VULKAN_HPP_NO_EXCEPTIONS
                $1
                #else
                $2
                #endif
                )"_seg.replace(std::nullopt, assertChecks, exceptionChecks);
        }

        // Generate a call.
        Segment simpleCall, enhancedCall, raiiPass;
        if (handle) (simpleCall, enhancedCall) << "m_" << handle->memberName << ", ";
        for (const auto& param : function.methodParams()) {
            Segment pass;
            if (param.pointers > 1 || param.kind != Var::Kind::VOID) {
                if (param.pointers && param.lenIfNotNull && param.kind != Var::Kind::VOID) // Array.
                    pass << param.prettyName << ".data()";
                else if (param.pointers && param.tag == Var::Tag::NOT_NULL) // Reference or local.
                    pass << "&" << param.prettyName;
                else if (param.pointers && !param.constant && param.kind != Var::Kind::OTHER && param.kind != Var::Kind::CHAR) // Optional.
                    pass << "static_cast<" << param.simpleType << ">(" << param.prettyName << ")";
            }
            if (!pass) pass << param.prettyName;
            if (param.kind == Var::Kind::VK || param.kind == Var::Kind::VMA) (simpleCall, enhancedCall) << // Cast Vk & Vma types.
                (param.pointers ? "reinterpret_cast<" : "static_cast<") << param.makeType(param.underlyingType, "") << ">(";
            simpleCall << param.name;
            enhancedCall << pass;
            if (param.kind == Var::Kind::VK || param.kind == Var::Kind::VMA) (simpleCall, enhancedCall) << ")"; // Cast Vk & Vma types.
            (simpleCall, enhancedCall) << ", ";
            if (param.paramType == Param::Type::OUTPUT || !param.deducedArrays.empty()) continue; // Skip outputs and deduced array sizes.
            if (function.secondHandleParam != &param) {
                if (combinedHandle && param.handle) raiiPass << (combining & constructor)["std::move("];
                raiiPass << param.prettyName;
                if (combinedHandle && param.handle) raiiPass << (combining & constructor)[")"];
            } else raiiPass << "m_" << function.handle->memberName;
            raiiPass << ", ";
        }
        "(" >>= (simpleCall, enhancedCall).pop() << ")";
        function.name >>= (simpleCall, enhancedCall);
        if (simpleReturn != "void")
            simpleReturn >>= " result = static_cast<" >>= simpleReturn >>= ">( " >>= (simpleCall, enhancedCall) << " )";
        simple.body << n << simpleCall << ";";
        enhanced.body << enhancedOutputs << n << enhancedCall << ";";

        raii.body << "return ";
        if (handle) raii.body << "m_" << handle->memberName << ".";
        else raii.body << "VMA_HPP_NAMESPACE::";
        raii.body << function.methodName << "(" << raiiPass.pop() << ")";
        if (hasRAIIVariant || combinedHandle) {
            if (hasRAIIVariant) raii.body <<
                (!combining)[",\n  detail::wrap<$0>($1detail::placeholder)"_seg.replace(raiiReturn, function.handle ? "*this, " : "")];
            if (combinedHandle) {
                Segment s = ",\n  detail::wrap<$0>("_seg.replace(raiiReturn);
                if (function.handle && !function.secondHandleParam) s << "*this, ";
                for (const auto& param : function.methodParams()) {
                    if (!param.handle) continue;
                    if (param.paramType == Param::Type::OUTPUT) s << "detail::placeholder";
                    else if (param.paramType == Param::Type::INPUT)
                        s << "std::move(" << (function.secondHandleParam == &param ? "*this" : param.prettyName) << ")";
                    s << ", ";
                }
                raii.body << combining[std::move(s.pop() << ")")];
            }
        }
        raii.body << ";";

        // Generate result check.
        if (resultCheck) enhanced.body << n << resultCheck;

        // Generate unique conversion.
        if (uniqueWrap) enhanced.body << unique[std::move(n >>= uniqueWrap)];

        // Generate return statement.
        if (simpleReturn != "void") simple.body << n << "return result;";
        if (simpleReturn != "void" || !outputs.empty()) {
            enhanced.body << n << "return ";
            if (resultValue == ResultValue::VALUE && outputs.empty()) enhanced.body << "result";
            else {
                if (resultValue == ResultValue::VALUE) enhanced.body << enhancedReturn << "(result, ";
                else if (resultValue == ResultValue::VALUE_TYPE)
                    enhanced.body << "VULKAN_HPP_NAMESPACE::detail::createResultValueType(result" << (outputs.empty() ? "" : ", ");
                if (!outputs.empty() || resultValue == ResultValue::NONE) {
                    if (hasUniqueVariant) enhanced.body << unique["std::move("];
                    enhanced.body << enhancedOutput;
                    if (hasUniqueVariant) enhanced.body << unique["Unique)"];
                }
                if (resultValue != ResultValue::NONE) enhanced.body << ")";
            }
            enhanced.body << ";";
        }

        // Append methods to the handle.
        auto buildMethod = [&](const Segment::Specialization variant, Handle* handle, const Token& name, const Method& method) {
            // Generate name.
            Segment methodName;
            if (handle && variant & !declaration) methodName << handle->name << "::";
            methodName << name;
            if (variant == (variant & unique)) methodName << "Unique";
            if (name == "free") "(" >>= methodName << ")"; // MSVC debug free workaround.
            // Make a signature.
            Segment result = "// wrapper "_seg << (variant == (variant & constructor) ? "constructor" : "function") <<
                " for command " << function.name << ", see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html";
            result << n << method.ret << " $0($1)"_seg;
            if (variant == (variant & combining) && handle && handle->owner) result << " &&";
            else if (handle && variant != (variant & constructor)) result << " const";
            if (resultValue == ResultValue::NONE || &method == &simple) result << " VULKAN_HPP_NOEXCEPT";
            else if (variant != (variant & constructor)) result << " VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS";
            if (variant & declaration) result << ";";
            else result << (variant == (variant & constructor) ? " :\n  $0($2) {}"_seg.replace(name) : " {\n  $2\n}"_seg);
            return std::move(result).replace(variant, methodName, method.params, method.body);
        };
        auto appendMethods = [&](Segment& dst, const Segment::Specialization variant, const Token& name) {
            dst << nn << navigate(function) << "#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE"_seg << n <<
                buildMethod(variant & !unique & !vectorAllocator, handle, name, enhanced);
            if (vecAllocName) dst << n <<
                buildMethod(variant & !unique & vectorAllocator, handle, name, enhanced);
            if (hasUniqueVariant) {
                dst << n << "#ifndef VULKAN_HPP_NO_SMART_HANDLE"_seg << n <<
                    buildMethod(variant & unique & !vectorAllocator, handle, name, enhanced);
                if (vecAllocName) dst << n <<
                    buildMethod(variant & unique & vectorAllocator, handle, name, enhanced);
                dst << n << "#endif"_seg;
            }
            dst << n << (signatureTransformed ? "#endif"_seg : "#else"_seg) << n <<
                buildMethod(variant & !unique, handle, name, simple);
            if (!signatureTransformed) dst << n << "#endif"_seg;
        };
        Handle& h = handle ? *handle : namespaceHandle;
        appendMethods(h.methodDecl, declaration, function.methodName);
        appendMethods(h.methodDef, !declaration, function.methodName);
        if (function.shortName) {
            appendMethods(h.methodDecl, declaration, function.shortName);
            appendMethods(h.methodDef, !declaration, function.shortName);
        }
        if (function.raiiName && (function.handle || hasRAIIVariant)) {
            if (function.name == "vmaCreateAllocator") { // Custom code for allocator creation.
                raiiPass = "instance, device, createInfo"_seg;
                (raii.params = R"(VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Instance const & instance,
                                  VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const & device,
                                  AllocatorCreateInfo
                                  )"_seg).pop() << constructor[" const &"] << " createInfo";
                raii.body = R"(
                if (createInfo.instance || createInfo.device || createInfo.pVulkanFunctions) {
                  VULKAN_HPP_NAMESPACE::detail::resultCheck(VULKAN_HPP_NAMESPACE::Result::eErrorValidationFailed, VMA_HPP_RAII_NAMESPACE_STRING
                    "::createAllocator: createInfo.instance, createInfo.device and createInfo.pVulkanFunctions must be null");
                  return VULKAN_HPP_NAMESPACE::detail::createResultValueType(VULKAN_HPP_NAMESPACE::Result::eErrorValidationFailed, Allocator(nullptr));
                }
                createInfo.instance = instance;
                createInfo.device = device;
                const VulkanFunctions functions = functionsFromDispatchers(*device.getDispatcher(), *instance.getDispatcher());
                createInfo.pVulkanFunctions = &functions;
                return VMA_HPP_NAMESPACE::createAllocator(createInfo),
                  detail::wrap<typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocator>::type>(device, detail::placeholder, createInfo.pAllocationCallbacks, device.getDispatcher());
                )"_seg;
            }
            Handle& r = function.handle ? *function.handle : namespaceHandle;
            r.raiiDecl << nn << navigate(function) << buildMethod(declaration & !constructor & !combining, function.handle, function.raiiName, raii);
            r.raiiDef << nn << navigate(function) << buildMethod(!declaration & !constructor & !combining, function.handle, function.raiiName, raii);
            if (combinedHandle) {
                r.raiiDecl << nn << navigate(function) << buildMethod(declaration & !constructor & combining, function.handle, function.raiiName, raii);
                r.raiiDef << nn << navigate(function) << buildMethod(!declaration & !constructor & combining, function.handle, function.raiiName, raii);
            }
            if (raiiOutputHandle || combinedHandle) {
                raii.ret.clear() << declaration["explicit"] << (!declaration)["VULKAN_HPP_INLINE"];
                if (function.handle) function.handle->name >>= " const & " >>= function.handle->memberName >>= "," >>= n >>= raii.params;
                raii.body.clear();
                if (function.handle) raii.body << function.handle->memberName << ".";
                else raii.body << "VMA_HPP_RAII_NAMESPACE::";
                raii.body << function.raiiName << "(" << raiiPass << ")";
                if (raiiOutputHandle) {
                    raiiOutputHandle->raiiConstructorDecl << navigate(function) << buildMethod(declaration & constructor & !combining, raiiOutputHandle, raiiOutputHandle->name, raii) << nn;
                    raiiOutputHandle->raiiConstructorDef << navigate(function) << buildMethod(!declaration & constructor & !combining, raiiOutputHandle, raiiOutputHandle->name, raii) << nn;
                }
                if (combinedHandle && !function.secondHandleParam) {
                    combinedHandle->raiiConstructorDecl << navigate(function) << buildMethod(declaration & constructor & combining, combinedHandle, combinedHandle->name, raii) << nn;
                    combinedHandle->raiiConstructorDef << navigate(function) << buildMethod(!declaration & constructor & combining, combinedHandle, combinedHandle->name, raii) << nn;
                }
            }
        }
    }

    // Generate handle declarations.
    Segment::Vector<7> content;
    auto& [forwardDecls, uniqueDecls, declarations, handleTraits, cppTypeTraits, uniqueTraits, definitions] = *content;
    for (const Symbol& s : symbols.structs) forwardDecls << n << navigate(s) << "struct " << s.name << ";";
    for (Handle& h : handleVector) {
        if (h.raiiOnly) continue;
        (forwardDecls, handleTraits, cppTypeTraits) << n << navigate(h);
        forwardDecls << "class " << h.name << ";";
        handleTraits << R"(
        template <> struct isVulkanHandleType<VMA_HPP_NAMESPACE::$0> {
          static VULKAN_HPP_CONST_OR_CONSTEXPR bool value = true;
        };
        )"_seg.replace(h.name);
        cppTypeTraits << R"(
        template <> struct CppType<Vma$0, VK_NULL_HANDLE> {
          using Type = VMA_HPP_NAMESPACE::$0;
        };
        )"_seg.replace(h.name);

        // Generate unique handle.
        if (h.destructor) {
            Segment owner, destructor;
            if (h.owner) {
                owner << "VMA_HPP_NAMESPACE::" << h.owner->name;
                destructor << "getOwner()." << h.destructor << "(t)";
            } else {
                owner << "void";
                destructor << "t." << h.destructor << "()";
            }
            uniqueDecls << n << navigate(h) << R"(
            using Unique$0 = VULKAN_HPP_NAMESPACE::UniqueHandle<$0, detail::Dispatcher>;
            )"_seg.replace(h.name);
            uniqueTraits << n << navigate(h) << R"(
            template <> class UniqueHandleTraits<VMA_HPP_NAMESPACE::$0, VMA_HPP_NAMESPACE::detail::Dispatcher> {
            public:
              class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<$1> {
              public:
                using UniqueBase::UniqueBase;
              protected:
                template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
                  $2;
                }
              };
            };
            )"_seg.replace(h.name, owner, destructor);
        }

        // Generate handle class.
        declarations << nn << navigate(h) << R"(
        // wrapper class for handle Vma$0, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_$2.html
        class $0 {
        public:
          using CType      = Vma$0;
          using NativeType = Vma$0;

        public:
          VULKAN_HPP_CONSTEXPR $0() VULKAN_HPP_NOEXCEPT = default;

          $0($0 const & rhs)             = default;
          $0 & operator=($0 const & rhs) = default;

        #if !defined( VULKAN_HPP_HANDLES_MOVE_EXCHANGE )
          $0($0 && rhs)             = default;
          $0 & operator=($0 && rhs) = default;
        #else
          $0($0 && rhs) VULKAN_HPP_NOEXCEPT : m_$1(exchange(rhs.m_$1, {})) {}
          $0 & operator=($0 && rhs) VULKAN_HPP_NOEXCEPT {
            m_$1 = exchange(rhs.m_$1, {});
            return *this;
          }
        #endif

          VULKAN_HPP_CONSTEXPR         $0(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
          VULKAN_HPP_TYPESAFE_EXPLICIT $0(Vma$0 $1) VULKAN_HPP_NOEXCEPT : m_$1($1) {}

        #if (VULKAN_HPP_TYPESAFE_CONVERSION == 1)
          $0& operator=(Vma$0 $1) VULKAN_HPP_NOEXCEPT {
            m_$1 = $1;
            return *this;
          }
        #endif

          $0& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
            m_$1 = {};
            return *this;
          }

          VULKAN_HPP_TYPESAFE_EXPLICIT operator Vma$0() const VULKAN_HPP_NOEXCEPT {
            return m_$1;
          }

          explicit operator bool() const VULKAN_HPP_NOEXCEPT {
            return m_$1 != VK_NULL_HANDLE;
          }

          bool operator!() const VULKAN_HPP_NOEXCEPT {
            return m_$1 == VK_NULL_HANDLE;
          }

          $3

        private:
          Vma$0 m_$1 = {};
        };
        )"_seg.replace(h.name, h.memberName, docLink(*h.name), h.methodDecl << navigate(h));
        definitions << nn << h.methodDef << navigate(h);
    }
    declarations << nn << namespaceHandle.methodDecl;
    definitions << nn << namespaceHandle.methodDef;
    content << navigate.reset;

    // Generate RAII handle declarations.
    Segment raiiContent, raiiSpecializations;
    for (Handle& h : handleVector) {
        raiiContent << n << navigate(h) << "class " << h.name << ";";
        raiiSpecializations << n << navigate(h) << R"(
        template <> struct isVulkanRAIIHandleType<VMA_HPP_NAMESPACE::VMA_HPP_RAII_NAMESPACE::$0> {
          static VULKAN_HPP_CONST_OR_CONSTEXPR bool value = true;
        };
        )"_seg.replace(h.name);
    }
    raiiContent << navigate.reset << nn << namespaceHandle.raiiDecl << navigate.reset;
    for (Handle& h : handleVector) {
        // Generate body pieces.
        Segment classTemplate, priv, constructors, moveInit, swap, operators, clear, release, getters;
        if (h.name == "Buffer" || h.name == "Image") {
            classTemplate = R"(
            // wrapper class for handle Vk$0 combined with VmaAllocation
            // see https://registry.khronos.org/vulkan/specs/latest/man/html/Vk$0.html
            // see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation.html
            class $0 : public VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0 {
            public:
              using CType   = Vk$0;
              using CppType = VULKAN_HPP_NAMESPACE::$0;

            public:
              $1

            private:
              friend class detail::Converter<$0>;
              explicit $0(Allocation&& allocation, VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0&& t) VULKAN_HPP_NOEXCEPT :
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0(std::move(t)),
                m_allocation(std::move(allocation)) {}
              explicit $0(const Allocator& allocator, VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0&& t, VMA_HPP_NAMESPACE::Allocation allocation) VULKAN_HPP_NOEXCEPT :
                $0(detail::wrap<Allocation>(allocator, allocation), std::move(t)) {}
              explicit $0(const Allocator& allocator, Allocation&& allocation, VULKAN_HPP_NAMESPACE::$0 t) VULKAN_HPP_NOEXCEPT :
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0(allocator.getDevice(), t, allocator.getAllocationCallbacks(), allocator.getDispatcher()),
                m_allocation(std::move(allocation)) {}
              explicit $0(const Allocator& allocator, std::pair<VMA_HPP_NAMESPACE::Allocation, VULKAN_HPP_NAMESPACE::$0> pair) VULKAN_HPP_NOEXCEPT :
                $0(allocator, detail::wrap<Allocation>(allocator, pair.first), pair.second) {}

              Allocation m_allocation = nullptr;
            };
            )"_seg;
            constructors = R"(
            $0(std::nullptr_t) : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0(nullptr) {}
            )"_seg;
            moveInit = R"(
            : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0(exchange(static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0&>(rhs), nullptr))
            , m_allocation(exchange(rhs.m_allocation, nullptr))
            )"_seg;
            swap = R"(
            std::swap(static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0&>(*this), static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0&>(rhs));
            std::swap(m_allocation, rhs.m_allocation);
            )"_seg;
            clear = R"(
            if (*m_allocation) {
              const auto allocator = m_allocation.getAllocator();
              const auto pair = release();
              allocator.destroy$0(pair.second, pair.first);
            }
            )"_seg;
            release = R"(
            std::pair<Allocation, VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0> split() VULKAN_HPP_NOEXCEPT {
              return { std::move(m_allocation), static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0&&>(*this) };
            }

            std::pair<VMA_HPP_NAMESPACE::Allocation, VULKAN_HPP_NAMESPACE::$0> release() {
              return { m_allocation.release(), VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::$0::release() };
            }
            )"_seg;
            getters = R"(
            VULKAN_HPP_NODISCARD const Allocation& getAllocation() const { return m_allocation; }
            )"_seg;
        } else if (h.name == "StatsString") {
            classTemplate = R"(
            // wrapper class for the stats string
            class $0 {
            public:
              using CType   = char*;
              using CppType = char*;

            public:
              $1

            private:
              friend class detail::Converter<StatsString>;
              using Destructor = void (*)(uint64_t, char*);
              template<class T, void (*destructor)(T, char*)>
              static void destroy(uint64_t owner, char* string) VULKAN_HPP_NOEXCEPT { destructor(reinterpret_cast<T>(owner), string); }
              template<class T> static uint64_t erase(T t) { return reinterpret_cast<uint64_t>(static_cast<typename T::CType>(t)); }
              $0(const VMA_HPP_NAMESPACE::Allocator allocator, char* string) VULKAN_HPP_NOEXCEPT
                : m_owner(erase(allocator))
                , m_string(string)
                , m_destructor(&destroy<VmaAllocator, &vmaFreeStatsString>) {}
              $0(const VMA_HPP_NAMESPACE::VirtualBlock virtualBlock, char* string) VULKAN_HPP_NOEXCEPT
                : m_owner(erase(virtualBlock))
                , m_string(string)
                , m_destructor(&destroy<VmaVirtualBlock, &vmaFreeVirtualBlockStatsString>) {}

              uint64_t   m_owner      = 0;
              char*      m_string     = nullptr;
              Destructor m_destructor = nullptr;
            };
            )"_seg;
            constructors = R"(
            $0(std::nullptr_t) {}
            )"_seg;
            moveInit = R"(
            : m_owner(exchange(rhs.m_owner, 0))
            , m_string(exchange(rhs.m_string, nullptr))
            , m_destructor(exchange(rhs.m_destructor, nullptr))
            )"_seg;
            swap = R"(
            std::swap(m_owner, rhs.m_owner);
            std::swap(m_string, rhs.m_string);
            std::swap(m_destructor, rhs.m_destructor);
            )"_seg;
            operators = R"(
            char* operator*() const VULKAN_HPP_NOEXCEPT { return m_string; }
            operator char*() const VULKAN_HPP_NOEXCEPT { return m_string; }
            )"_seg.replace(h.memberName);
            clear = R"(
            if (m_string) m_destructor(m_owner, m_string);
            m_owner = 0;
            m_string = nullptr;
            m_destructor = nullptr;
            )"_seg;
            release = R"(
            char* release() {
              m_owner = 0;
              m_destructor = nullptr;
              return exchange(m_string, nullptr);
            }
            )"_seg;
        } else {
            // Generate members and constructors.
            struct Member {
                Segment type;
                Token name;
            };
            std::vector<Member> members;
            if (h.name == "DefragmentationContext") h.destructor = "endDefragmentation"; // Custom destructor for DefragmentationContext.
            if (h.name == "Allocator") { // Custom members for Allocator.
                members = {
                    Member { "VULKAN_HPP_NAMESPACE::Device"_seg, "device" },
                    Member { "VMA_HPP_NAMESPACE::Allocator"_seg, "allocator" },
                    Member { "const VULKAN_HPP_NAMESPACE::AllocationCallbacks *"_seg, "allocationCallbacks" },
                    Member { "VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const *"_seg, "dispatcher" }
                };
            } else {
                if (h.owner) members.push_back(Member { "VMA_HPP_NAMESPACE::"_seg << h.owner->name, h.owner->memberName });
                members.push_back(Member { "VMA_HPP_NAMESPACE::"_seg << h.name, h.memberName });
            }

            classTemplate = R"(
            // wrapper class for handle Vma$0, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_$3.html
            class $0 {
            public:
              using CType   = Vma$0;
              using CppType = VMA_HPP_NAMESPACE::$0;

            public:
              $1

            private:
              $2
            };
            )"_seg.replace(std::nullopt, std::nullopt, std::nullopt, docLink(*h.name));
            operators = R"(
            CppType const & operator*() const & VULKAN_HPP_NOEXCEPT { return m_$0; }
            CppType const && operator*() const && VULKAN_HPP_NOEXCEPT { return std::move(m_$0); }
            operator CppType() const VULKAN_HPP_NOEXCEPT { return m_$0; }
            )"_seg.replace(h.memberName);
            clear << "if (m_" << h.memberName << ") m_" << (h.owner ? h.owner->memberName : h.memberName) << "." << h.destructor << "(";
            if (h.owner) clear << "m_" << h.memberName;
            clear << ");";
            priv << "friend class detail::Converter<$0>;\nexplicit $0("_seg;
            for (const auto& [type, name] : members) priv << type << " " << name << ", ";
            priv.pop() << ") VULKAN_HPP_NOEXCEPT :" << n << "  ";
            for (const auto& [type, name] : members) priv << "m_" << name << "(" << name << ")" << ", ";
            priv.pop() << " {}" << nn;
            for (const auto& [type, name] : members) {
                priv << type << " m_" << name << " = {};" << n;
                moveInit << (moveInit ? ", m_" : ": m_") << name << "(exchange(rhs.m_" << name << ", {}))" << n;
                swap << "std::swap(m_" << name << ", rhs.m_" << name << ");" << n;
                clear << n << "m_" << name << " = nullptr;";
                if (h.memberName != name) {
                    release << n << "m_" << name << " = nullptr;";
                    getters << nn << "VULKAN_HPP_NODISCARD " << type << " get" << name.capitalize() << "() const { return m_" << name << "; }";
                }
            }
            constructors = R"(
            $0(std::nullptr_t) {}
            )"_seg;
            release = R"(
            CppType release() {
              $0
              return exchange(m_$1, nullptr);
            }
            )"_seg.replace(release, h.memberName);
        }

        // Generate class body.
        Segment body = R"(
        #if !defined( VULKAN_HPP_NO_EXCEPTIONS )
        $1
        #endif

        $2
        ~$0() { clear(); }

        $0() = delete;
        $0($0 const &) = delete;

        $0($0 && rhs) VULKAN_HPP_NOEXCEPT
          $3 {}

        $0& operator=($0 const &) = delete;
        $0& operator=($0 && rhs) VULKAN_HPP_NOEXCEPT {
          if (this != &rhs) {
            $4
          }
          return *this;
        }

        $5

        void clear() VULKAN_HPP_NOEXCEPT {
          $6
        }

        $7

        $8

        void swap($0 & rhs) VULKAN_HPP_NOEXCEPT {
          $4
        }
        )"_seg.replace(std::nullopt, h.raiiConstructorDecl.pop() << navigate(h), constructors, moveInit.pop(), swap.pop(), operators, clear, release, getters);

        // Generate handle class.
        raiiContent << nn << navigate(h) << std::move(classTemplate).replace(h.name, body << h.raiiDecl << navigate(h), priv);
    }
    raiiContent << navigate.reset << nn << namespaceHandle.raiiDef << navigate.reset;
    for (Handle& h : handleVector) raiiContent << nn << h.raiiDef;
    raiiContent << navigate.reset << nn << "#if !defined( VULKAN_HPP_NO_EXCEPTIONS )"_seg << n;
    for (Handle& h : handleVector) raiiContent << h.raiiConstructorDef;
    raiiContent.pop() << navigate.reset << n << "#endif"_seg;


    // Generate files.
    R"(
    namespace VMA_HPP_NAMESPACE {
      $0

      namespace detail { class Dispatcher; } // VMA dispatcher is a no-op.
    #ifndef VULKAN_HPP_NO_SMART_HANDLE
      using UniqueBuffer = VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Buffer, detail::Dispatcher>;
      using UniqueImage = VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Image, detail::Dispatcher>;
      $1
    #endif

      $2

    #ifndef VULKAN_HPP_NO_SMART_HANDLE
      namespace detail {
        template <typename OwnerType> class UniqueBase {
        public:
          UniqueBase() = default;
          UniqueBase(OwnerType owner) VULKAN_HPP_NOEXCEPT : m_owner(owner) {}
          OwnerType getOwner() const VULKAN_HPP_NOEXCEPT { return m_owner; }
        private:
          OwnerType m_owner = {};
        };
        template <> class UniqueBase<void> {};
      }
    #endif
    }

    namespace VULKAN_HPP_NAMESPACE {
      $3

    #if (VK_USE_64_BIT_PTR_DEFINES == 1)
      $4
    #endif

    #ifndef VULKAN_HPP_NO_SMART_HANDLE
      template <> class UniqueHandleTraits<Buffer, VMA_HPP_NAMESPACE::detail::Dispatcher> {
      public:
        class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<VMA_HPP_NAMESPACE::Allocator> {
        public:
          using UniqueBase::UniqueBase;
        protected:
          template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
            vmaDestroyBuffer(getOwner(), t, nullptr);
          }
        };
      };
      template <> class UniqueHandleTraits<Image, VMA_HPP_NAMESPACE::detail::Dispatcher> {
      public:
        class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<VMA_HPP_NAMESPACE::Allocator> {
        public:
          using UniqueBase::UniqueBase;
        protected:
          template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
            vmaDestroyImage(getOwner(), t, nullptr);
          }
        };
      };
      $5
    #endif
    }
    )"_seg.replace(forwardDecls, uniqueDecls, declarations, handleTraits, cppTypeTraits, uniqueTraits)
        .resolve(source.tree).generateHpp("handles");
    R"(
    namespace VMA_HPP_NAMESPACE {
      $0
    }
    )"_seg.replace(definitions).resolve(source.tree).generateHpp("funcs");
    R"(
    #ifndef VMA_HPP_CXX_MODULE
    #include <vulkan/vulkan_raii.hpp>
    #endif

    #ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    #pragma warning(disable : 4834) // MSVC thinks we are discarding chained return values, like foo(), detail::wrap<...>(...)
    namespace VMA_HPP_NAMESPACE {
      namespace VMA_HPP_RAII_NAMESPACE {
        namespace detail {
          template<int N, int... I> struct Seq : Seq<N-1, N-1, I...> {};
          template<int... I> struct Seq<0, I...> {};
          struct Placeholder {} VULKAN_HPP_CONSTEXPR_INLINE placeholder;

          // Base converter, creates an object (delegating to the final converter), substituting argument placeholders.
          template<class Dst, class Wrapper = void, class... Src> struct Converter {
            Dst convert(Src&&... src) const {
              return create(static_cast<const Wrapper&>(*this), std::forward<Src>(src)...);
            }
          private:
            template<class... Args> Dst create(const std::tuple<Args...>& args, Src&&... src) const {
              return create(Seq<sizeof...(Args)>(), args, std::forward<Src>(src)...);
            }
            template<class... Args, int... I> Dst create(Seq<0, I...>, const std::tuple<Args...>& args, Src&&... src) const {
              return Converter<Dst>::create(convertParam(std::forward<Args>(std::get<I>(args)), std::forward<Src>(src)...)...);
            }
            template<class T> static T&& convertParam(T&& t, Src&&...) { return std::forward<T>(t); }
            template<class T=typename std::enable_if<true, Src...>::type> // Only when Src exists.
            static T&& convertParam(Placeholder, Src&&... src) { return std::forward<Src...>(src...); }
          };

          // Final converter, creates an object from the provided arguments.
          template<class Dst> struct Converter<Dst> {
            template<class... Args> static Dst create(Args&&... args) { return Dst(std::forward<Args>(args)...); }
          };

          // std::vector converter.
          template<class Dst, class DstAllocator, class Wrapper, class Src, class SrcAllocator>
          struct Converter<std::vector<Dst, DstAllocator>, Wrapper, std::vector<Src, SrcAllocator>> : Converter<Dst, Wrapper, Src> {
            std::vector<Dst, DstAllocator> convert(std::vector<Src, SrcAllocator>&& src) const {
              std::vector<Dst, DstAllocator> result;
              result.reserve(src.size());
              for (Src& s : src) result.push_back(Converter<Dst, Wrapper, Src>::convert(std::forward<Src>(s)));
              return std::move(result);
            }
          };

          // ResultValue converter.
          template<class Dst, class Wrapper, class... Src>
          struct Converter<VULKAN_HPP_NAMESPACE::ResultValue<Dst>, Wrapper, VULKAN_HPP_NAMESPACE::ResultValue<Src>...> : Converter<Dst, Wrapper, Src...> {
            VULKAN_HPP_NAMESPACE::ResultValue<Dst> convert(VULKAN_HPP_NAMESPACE::Result result, Src&&... src) const {
              return VULKAN_HPP_NAMESPACE::ResultValue<Dst>(result, result < VULKAN_HPP_NAMESPACE::Result::eSuccess ?
                none() : Converter<Dst, Wrapper, Src...>::convert(std::forward<Src>(src)...));
            }
            template<class T=typename std::enable_if<true, Src...>::type> // Only when Src exists.
            VULKAN_HPP_NAMESPACE::ResultValue<Dst> convert(VULKAN_HPP_NAMESPACE::ResultValue<T>&& src) const {
              return convert(src.result, std::forward<Src...>(src.value));
            }
          private:
            template<class T=Dst> static typename std::enable_if<std::is_default_constructible<T>::value, T>::type none() { return Dst(); }
            template<class T=Dst> static typename std::enable_if<!std::is_default_constructible<T>::value, T>::type none() { return Dst(nullptr); }
          };
          template<class Dst, class Wrapper> // Converting from plain vk::Result.
          struct Converter<VULKAN_HPP_NAMESPACE::ResultValue<Dst>, Wrapper, VULKAN_HPP_NAMESPACE::Result> :
                 Converter<VULKAN_HPP_NAMESPACE::ResultValue<Dst>, Wrapper> {};

          // vk::raii::Buffer and vk::raii::Image converters.
          template<class T> struct VulkanRAIIResourceConverter : private T {
            using T::T;
            template<class Allocator> static T create(const Allocator& alloc, typename T::CppType t) {
              return VulkanRAIIResourceConverter(alloc.getDevice(), t, alloc.getAllocationCallbacks(), alloc.getDispatcher());
            }
          };
          template<> struct Converter<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer> :
          VulkanRAIIResourceConverter<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer> {};
          template<> struct Converter<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image> :
          VulkanRAIIResourceConverter<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image> {};

          // Wrapper, converting Src with additional Args into Dst.
          template<class Src, class Dst, class... Args> struct Wrapper :
          std::tuple<Args&&...>, Converter<Dst, Wrapper<Src, Dst, Args...>, Src> {
            explicit Wrapper(Wrapper<void, Dst, Args...>&& base, Src&& src) :
              std::tuple<Args&&...>(std::move(base)), src(std::forward<Src>(src)) {}
            operator Dst() && { return this->convert(std::forward<Src>(src)); }
          private:
            Src src;
          };

          // Wrapper without an input, converts Args into Dst.
          template<class Dst, class... Args> struct Wrapper<void, Dst, Args...> :
          std::tuple<Args&&...>, Converter<Dst, Wrapper<void, Dst, Args...>> {
            explicit Wrapper(Args&&... args) : std::tuple<Args&&...>(std::forward<Args>(args)...) {}
            template<class Src> friend Wrapper<Src, Dst, Args...> operator,(Src&& src, Wrapper&& w) {
              return Wrapper<Src, Dst, Args...>(std::move(w), std::forward<Src>(src));
            }
            operator Dst() && { return this->convert(); }
          };

          template<class Dst, class... Args> Wrapper<void, Dst, Args...> wrap(Args&&... args) {
            return Wrapper<void, Dst, Args...>(std::forward<Args>(args)...);
          }
        }

        #if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
        using VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::operator<=>;
        #else
        using VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::operator<;
        #endif
        using VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::operator==;
        using VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::operator!=;
        using VULKAN_HPP_NAMESPACE::exchange;
        $0
      }
    }
    namespace VULKAN_HPP_NAMESPACE {
      namespace VULKAN_HPP_RAII_NAMESPACE {
        $1
      }
    }
    #endif
    )"_seg.replace(raiiContent, raiiSpecializations << navigate.reset).resolve(source.tree).generateHpp("raii");
}

void generateStaticAssertions(const ConditionalTree& tree, const Symbols& symbols) {
    Segment content;
    for (const Symbol& s : symbols.structs) {
        content << nn << navigate(s) << R"(
        VULKAN_HPP_STATIC_ASSERT(sizeof(VMA_HPP_NAMESPACE::$0) == sizeof(Vma$0), "struct and wrapper have different size!");
        VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<VMA_HPP_NAMESPACE::$0>::value, "struct wrapper is not a standard layout!");
        VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<VMA_HPP_NAMESPACE::$0>::value, "$0 is not nothrow_move_constructible!");
        )"_seg.replace(s.name);
    }
    for (const Symbol& h : symbols.handles) {
        content << nn << navigate(h) << R"(
        VULKAN_HPP_STATIC_ASSERT(sizeof(VMA_HPP_NAMESPACE::$0) == sizeof(Vma$0), "handle and wrapper have different size!");
        VULKAN_HPP_STATIC_ASSERT(std::is_copy_constructible<VMA_HPP_NAMESPACE::$0>::value, "$0 is not copy_constructible!");
        VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<VMA_HPP_NAMESPACE::$0>::value, "$0 is not nothrow_move_constructible!");
        )"_seg.replace(h.name);
    }
    std::move("#include <vk_mem_alloc.hpp>" >>= content << navigate.reset).resolve(tree).generateHpp("static_assertions");
}

void generateModule(const ConditionalTree& tree, const Symbols& symbols) {
    Segment exports, uniqueExports, raiiExports, specializations, raiiSpecializations;

    // Generate export statements.
    for (const auto* list : { &symbols.enums, &symbols.structs, &symbols.handles, &symbols.functions })
        for (const Symbol& t : *list)
            exports << n << navigate(t) << "using VMA_HPP_NAMESPACE::" << t.name << ";";
    for (const Symbol& t : symbols.handles.unique)
        uniqueExports << n << navigate(t) << "using VMA_HPP_NAMESPACE::Unique" << t.name << ";";
    for (const Symbol& t : symbols.functions.unique)
        uniqueExports << n << navigate(t) << "using VMA_HPP_NAMESPACE::" << t.name << "Unique;";
    for (const auto* list : { &symbols.handles.raii, &symbols.functions.raii })
        for (const Symbol& t : *list)
            raiiExports << n << navigate(t) << "using VMA_HPP_RAII_NAMESPACE::" << t.name << ";";

    // Some workarounds for compilation errors on MSVC...

    // error C2678: binary '|': no operator found which takes a left-hand operand of type 'const vma::AllocationCreateFlagBits' (or there is no acceptable conversion)
    for (const Symbol& t : symbols.enums)
        if (endsWith(*t.name, "FlagBits"))
            specializations << n << navigate(t) << "template<> struct FlagTraits<VMA_HPP_NAMESPACE::" << t.name << ">;";

    // fatal error C1116: unrecoverable error importing module 'vk_mem_alloc_hpp'.  Specialization of 'vma::operator ==' with arguments 'vma::Pool, 0'
    for (const Symbol& t : symbols.handles)
        specializations << n << navigate(t) << "template<> struct isVulkanHandleType<VMA_HPP_NAMESPACE::" << t.name << ">;";

    // error C2027: use of undefined type 'vk::UniqueHandleTraits<Type,Dispatch>'
    for (const Symbol& t : symbols.handles.unique)
        specializations << n << navigate(t) << "template<> class UniqueHandleTraits<VMA_HPP_NAMESPACE::" << t.name << ", VMA_HPP_NAMESPACE::detail::Dispatcher>;";

    // error C2676: binary '==': 'const vma::raii::Allocator' does not define this operator or a conversion to a type acceptable to the predefined operator
    for (const Symbol& t : symbols.handles.raii)
        raiiSpecializations << n << navigate(t) << "template<> struct isVulkanRAIIHandleType<VMA_HPP_NAMESPACE::VMA_HPP_RAII_NAMESPACE::" << t.name << ">;";

    (exports, raiiExports, specializations, raiiSpecializations) << navigate.reset;

    // Don't forget Buffer and Image.
    uniqueExports << n << "using VMA_HPP_NAMESPACE::UniqueBuffer;" <<
                     n << "using VMA_HPP_NAMESPACE::UniqueImage;";
    specializations << n << "template<> class UniqueHandleTraits<Buffer, VMA_HPP_NAMESPACE::detail::Dispatcher>;"  <<
                       n << "template<> class UniqueHandleTraits<Image, VMA_HPP_NAMESPACE::detail::Dispatcher>;";

    R"(// Generated from the Vulkan Memory Allocator (vk_mem_alloc.h).
    module;
    #define VMA_HPP_CXX_MODULE
    #define VMA_IMPLEMENTATION
    #include <vk_mem_alloc.hpp>
    #include <vk_mem_alloc_raii.hpp>
    export module vk_mem_alloc_hpp;
    export import vulkan_hpp;

    export namespace VMA_HPP_NAMESPACE {
    #ifndef VULKAN_HPP_NO_TO_STRING
      using VMA_HPP_NAMESPACE::to_string;
    #endif
      using VMA_HPP_NAMESPACE::functionsFromDispatchers;
      using VMA_HPP_NAMESPACE::functionsFromDispatcher;
      using VMA_HPP_NAMESPACE::operator&;
      using VMA_HPP_NAMESPACE::operator|;
      using VMA_HPP_NAMESPACE::operator^;
      using VMA_HPP_NAMESPACE::operator~;
      using VMA_HPP_NAMESPACE::operator<;
      using VMA_HPP_NAMESPACE::operator<=;
      using VMA_HPP_NAMESPACE::operator>;
      using VMA_HPP_NAMESPACE::operator>=;
      using VMA_HPP_NAMESPACE::operator==;
      using VMA_HPP_NAMESPACE::operator!=;
      $0
      #ifndef VULKAN_HPP_NO_SMART_HANDLE
      $1
      #endif
      #ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
      namespace VMA_HPP_RAII_NAMESPACE {
        #if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
        using VMA_HPP_RAII_NAMESPACE::operator<=>;
        #else
        using VMA_HPP_RAII_NAMESPACE::operator<;
        #endif
        using VMA_HPP_RAII_NAMESPACE::operator==;
        using VMA_HPP_RAII_NAMESPACE::operator!=;
        $2
      }
      #endif
    }

    module : private;
    namespace VULKAN_HPP_NAMESPACE {
      // This is needed for template specializations to be visible outside the module when importing vulkan_hpp (is this a MSVC bug?).
      $3
      #ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
      namespace VULKAN_HPP_RAII_NAMESPACE {
        $4
      }
      #endif
    }
    )"_seg.replace(exports, uniqueExports, raiiExports, specializations, raiiSpecializations).resolve(tree).generate("vk_mem_alloc.cppm");
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

        Symbols symbols;
        generateEnums(source, symbols);
        generateStructs(source, symbols);
        generateHandles(source, symbols);
        generateStaticAssertions(source.tree, symbols);
        generateModule(source.tree, symbols);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}