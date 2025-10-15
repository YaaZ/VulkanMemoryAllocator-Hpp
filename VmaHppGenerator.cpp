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

template<class, class=void> struct HasStdGet : std::bool_constant<false> {};
template<class T> struct HasStdGet<T, std::void_t<decltype(std::get<0>(std::declval<T>()))>> : std::bool_constant<true> {};

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

    struct NodeInfo {
        std::optional<std::string_view> view;
        int indent;
    };
    static std::optional<NodeInfo> info(const Node& node) {
        if (const auto* n = std::get_if<StringViewNode>(&node))
            return {{ std::string_view(n->data, static_cast<unsigned int>(n->length)), n->indent }};
        if (const auto* n = std::get_if<StringNode>(&node))
            return {{ n->data->string, n->data->indent }};
        if (const auto* n = std::get_if<WildcardNode>(&node))
            return {{ std::nullopt, n->indent }};
        return std::nullopt;
    }

    template<class T, std::size_t> using ArrayTupleElement = T;
    template<class T, std::size_t... I>
    static auto ArrayTupleType(std::index_sequence<I...>) -> std::tuple<ArrayTupleElement<T, I>...>* { return nullptr; }
    template<class T, std::size_t S> using ArrayTuple = std::remove_pointer_t<decltype(ArrayTupleType<T>(std::make_index_sequence<S>()))>;

    template<int Size, class T, template<int, class> class Self> class VectorBase : public ArrayTuple<T, Size> {
        template<std::size_t... I> Self<Size + 1, T> join(T&& o, std::index_sequence<I...>) && {
            return { std::forward<T>(std::get<I>(*this))..., std::forward<T>(o) };
        }
    public:
        using ArrayTuple<T, Size>::ArrayTuple;
        auto& operator*() { return static_cast<ArrayTuple<T, Size>&>(*this); }
        Self<Size + 1, T> operator,(T&& o) && { return std::move(*this).join(std::forward<T>(o), std::make_index_sequence<Size>()); }
        Self<2, Self<Size, T>> operator,(Self<Size, T>&& o) && { return { static_cast<Self<Size, T>&&>(*this), std::move(o) }; }
    };

public:
    template<int Size, class T = Segment> class Vector : public VectorBase<Size, T, Vector> {
        template<std::size_t, class Ts, class=std::enable_if_t<!HasStdGet<Ts>::value>>
        static const auto& element(const Ts& t) { return t; }
        template<std::size_t>
        static Segment element(const Segment& t) { return t; }
        template<std::size_t, class... Ts>
        static const auto& element(const std::variant<Ts...>& t) { return t; }
        template<std::size_t I, class... Ts>
        static const auto& element(const std::tuple<Ts...>& t) { return std::get<I>(t); }
        template<std::size_t I, class... Ts>
        static const auto& element(const std::pair<Ts...>& t) { return std::get<I>(t); }

        template<bool Back, class Arg, std::size_t I = 0> void plus(const Arg& arg) {
            if constexpr (Back) std::get<I>(*this) + element<I>(arg);
            else element<I>(arg) + std::get<I>(*this);
            if constexpr (I < Size - 1) plus<Back, Arg, I + 1>(arg);
        }
    public:
        using VectorBase<Size, T, Vector>::VectorBase;
        template<std::size_t I = 0> Vector& pop() {
            std::get<I>(*this).pop();
            if constexpr (I < Size - 1) pop<I + 1>();
            return *this;
        }
        template<class Arg> friend Vector& operator+(Vector& v, const Arg& arg) { v.template plus<true>(arg); return v; }
        template<class Arg> friend Vector& operator+(const Arg& arg, Vector& v) { v.template plus<false>(arg); return v; }
        template<class Arg> friend Vector&& operator+(Vector&& v, const Arg& arg) { v.template plus<true>(arg); return std::move(v); }
        template<class Arg> friend Vector&& operator+(const Arg& arg, Vector&& v) { v.template plus<false>(arg); return std::move(v); }
    };

    class Token : public Node {
    public:
        template<int Size, class T = Token> struct Vector : VectorBase<Size, T, Vector> {
            using VectorBase<Size, T, Vector>::VectorBase;
        };
        Token() : Node(StringViewNode { nullptr, 0, 0 }) {}
        Token(const char* s, const int indent = 0) : Node(StringViewNode { s, static_cast<int>(std::strlen(s)), indent }) {}
        template<std::size_t N> Token(const char(&s)[N], int indent = 0) : Node(StringViewNode { s, N - 1, indent }) {}
        Token(const std::string_view& s, const int indent = 0) : Node(StringViewNode { s.data(), static_cast<int>(s.length()), indent }) {}
        Token(const std::string& s, const int indent = 0) : Node(StringNode { std::make_shared<StringNode::Data>(StringNode::Data { s, indent }) }) {}
        Vector<2, Token&> operator,(Token& o) { return { *this, o }; }
        explicit operator std::string() const { return std::string(**this); }
        std::size_t length() const { return (**this).length(); }
        explicit operator bool() const { return length() > 0; }
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
    Segment& pop() { if (!nodes.empty()) nodes.pop_back(); return *this; }
    /**
     * Has no text or wildcard tokens (navigation and breaks are ignored).
     */
    bool blank() const {
        for (const auto& n : nodes) if (const auto i = info(n); i && (!i->view || !i->view->empty())) return false;
        return true;
    }

    Vector<2, Segment&> operator,(Segment& o) { return { *this, o }; }
    friend Segment& operator+(Segment& s, const Token& o) { s.nodes.emplace_back(o); return s; }
    friend Segment& operator+(const Token& o, Segment& s) { s.nodes.emplace(s.nodes.begin(), o); return s; }
    friend Segment& operator+(Segment& s, const Node& o) { s.nodes.emplace_back(o); return s; }
    friend Segment& operator+(const Node& o, Segment& s) { s.nodes.emplace(s.nodes.begin(), o); return s; }

    friend Segment& operator+(Segment& s, Segment&& o) {
        // Calculate the length of the current line.
        int indent = 0, length = 0, fill = 0;
        for (int j = s.nodes.size() - 1; j >= 0; --j) {
            const auto i = info(s.nodes[j]);
            if (!i) break;
            const int l = i->view ? static_cast<int>(i->view->length()) : 0;
            fill += indent - i->indent - l;
            length += l;
            indent = i->indent;
            if (fill < 0) fill = 0;
        }
        indent += length + fill;
        length = static_cast<int>(s.nodes.size());
        // Append nodes.
        s.nodes.reserve(s.nodes.size() + o.nodes.size());
        std::move(o.nodes.begin(), o.nodes.end(), std::back_inserter(s.nodes));
        // Indent new nodes.
        for (int j = length; j < s.nodes.size(); ++j) indentNode(s.nodes[j], indent);
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
    int sourcePosition;
    Symbol(const Symbol&) = default;
    template<class... T> explicit Symbol(Token name, const T&... t) : name(std::move(name)), sourcePosition((navigate.sourcePosition(t) + ...)) {}
};
using Symbols = std::vector<Symbol>;

/**
 * Parsed variable declaration (field or function parameter).
 */
class Var {
public:
    Token name; // Original name
    Token prettyName; // Name with p* prefix removed for pointers
    Token type; // Full converted C++ type
    Token prettyType; // Full converted C++ type with the last pointer stripped
    Token originalType; // Original type
    Token underlyingType; // Original type without array size, const and pointers
    Token lenIfNotNull;
    enum class Kind : std::uint8_t { OTHER, VOID, CHAR, PFN, VK, VMA, ARRAY } kind;
    enum class Tag : std::uint8_t { NONE, NULLABLE, NOT_NULL } tag;
    std::uint8_t pointers;
    bool constant;
    int sourcePosition;
    int deducedArraySize = -1; // Array params point to the corresponding size param (if found), or -1
    std::vector<int> deducedArrays; // Size param points to corresponding arrays.

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

        underlyingType = group(2);
        std::string type { underlyingType }, originalType { underlyingType };
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
        if (constant && pointers > 0) {
            type = "const " + type;
            originalType = "const " + originalType;
        }
        if (pointers == 2) {
            constant = false; // double pointer, then first-level pointer is not const
            type += "**";
            originalType += "**";
        } else if (pointers == 1) {
            type += "*";
            originalType += "*";
        }
        name = group(9);
        prettyName = pointers && name.length() > 0 && (*name)[0] == 'p' ? name.substr(1).capitalize(false) : name;

        std::string_view arr = trim(group(10));
        originalType += arr;
        while (!(arr = trim(arr)).empty()) {
            const auto i = arr.find_last_of('['); // assert(i != npos);
            std::string dim { trim(arr.substr(i + 1, arr.length() - i - 2)) };
            type = "std::array<" + type + ", " + dim + ">";
            arr = arr.substr(0, i);
            kind = Kind::ARRAY;
        }
        this->type = type;
        this->prettyType = pointers ? this->type.substr(0, this->type.length() - 1) : this->type;
        this->originalType = originalType;

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
        // Find dependencies of array sizes.
        for (int i = 0; i < result.size(); ++i) {
            // Ignore void* - those should not be represented as arrays.
            if (auto& a = result[i]; a.lenIfNotNull && (a.pointers > 1 || a.kind != Kind::VOID)) {
                for (int j = 0; j < result.size(); ++j) {
                    if (auto& s = result[j]; *a.lenIfNotNull == *s.name) {
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

Symbols generateEnums(const Source& source) {
    Symbols enums;
    const std::regex typedefPattern { R"(typedef\s+enum\s+Vma(\w+))" };
    const std::regex entryPattern { R"((VMA_\w+)[^,}]*)" };

    Segment::Vector<3> enumPieces;
    auto& [content, toString, flagTraits] = *enumPieces;
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
            flagTraits + nn + navigate(match) + R"(

            template<> struct FlagTraits<VMA_HPP_NAMESPACE::$0> {
              using WrappedType = Vma$0;
              static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true, isVmaBitmask = true;
              static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<VMA_HPP_NAMESPACE::$0> allFlags =
                $1;
            };
            )"_seg.replace(name, allFlags);

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

    // Generate operators, some in vk:: are not found by ADL.
    Segment opTemplate = R"(
    template <typename BitType, typename std::enable_if<VULKAN_HPP_NAMESPACE::FlagTraits<BitType>::isVmaBitmask, bool>::type = true>
    VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR $1 operator$0($2) VULKAN_HPP_NOEXCEPT {
      return $3;
    }
    )"_seg;
    Segment operators = Segment(opTemplate)
        .replace("~", "VULKAN_HPP_NAMESPACE::Flags<BitType>", "BitType bit", "~(VULKAN_HPP_NAMESPACE::Flags<BitType>(bit))");
    opTemplate.replace("$0"_seg, "$1"_seg, "BitType lhs, BitType rhs", "VULKAN_HPP_NAMESPACE::Flags<BitType>(lhs) $0 rhs"_seg);
    for (const auto& op : { "&", "|", "^" })
        operators + Segment(opTemplate).replace(op, "VULKAN_HPP_NAMESPACE::Flags<BitType>");
    operators + "#if !defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )"_seg;
    for (const auto& op : { "<", "<=", ">", ">=", "==", "!=" })
        operators + Segment(opTemplate).replace(op, "bool");
    operators + "#endif"_seg;

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

    Segment content;
    for (const auto& match : iterate(source, typedefPattern)) {
        Token name = group(source, match, 1); // E.g. DeviceMemoryCallbacks
        structs.emplace_back(name, match);

        const auto begin = match.position() + match.length();
        const auto body = source.substr(begin, source.find("}", begin) - begin);
        std::vector<Var> members = Var::parse(body, begin);

        Token constexprToken = "VULKAN_HPP_CONSTEXPR";
        Segment::Vector<9> memberPieces;
        auto& [constructorParams, constructorInitializer, enhancedParams, enhancedInitializer,
               setters, reflectTypes, reflectTie, comparison, fields] = *memberPieces;
        bool containsPFN = false, hasEnhancedConstructor = false;
        for (const Var& member : members) {
            Token storageType = member.type;
            if (member.kind == Var::Kind::ARRAY) {
                static const std::regex regex { "std::array" };
                storageType = std::regex_replace(std::string(member.type), regex, "VULKAN_HPP_NAMESPACE::ArrayWrapper1D");
                constexprToken = "VULKAN_HPP_CONSTEXPR_14";
            } else containsPFN |= member.kind == Var::Kind::PFN;
            memberPieces + navigate(member);

            // Generate constructor parameter and setter.
            if (!constructorParams.blank())
                (constructorParams, constructorInitializer, reflectTypes, reflectTie) + n + Token(", ", -2);
            constructorParams + member.type + " " + member.name + "_ = {}";
            constructorInitializer + member.name + " { " + member.name + "_ }";
            setters + R"(
            VULKAN_HPP_CONSTEXPR_14 $0& set$3($2 $1_) VULKAN_HPP_NOEXCEPT {
              $1 = $1_;
              return *this;
            }

            )"_seg.replace(name, member.name, member.type, member.name.capitalize());

            // Generate enhanced constructor parameter and setter.
            if (member.deducedArrays.empty()) { // Skip deduced size params.
                if (!enhancedParams.blank()) (enhancedParams, enhancedInitializer) + n + Token(", ", -2);
                Token defaultAssignment = hasEnhancedConstructor ? " = {}" : ""; // No default assignments till the first enhanced param (inclusive).
                if (member.deducedArraySize != -1) {
                    hasEnhancedConstructor = true;
                    const auto& size = members[member.deducedArraySize];
                    if (size.deducedArrays.size() > 1) throw std::runtime_error("Deducing size from multiple arrays"); // Not implemented.
                    enhancedParams + "VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<" + member.prettyType + "> const & " + member.prettyName + "_";
                    enhancedInitializer + size.name + " { static_cast<" + size.type + ">(" + member.prettyName + "_.size()) }";
                    enhancedInitializer + n + Token(", ", -2) + member.name + " { " + member.prettyName + "_.data() }";
                    setters + R"(
                    #if !defined( VULKAN_HPP_DISABLE_ENHANCED_MODE )
                    $0& set$1(VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<$5> const & $2_) VULKAN_HPP_NOEXCEPT {
                      $4 = static_cast<$6>($2_.size());
                      $3 = $2_.data();
                      return *this;
                    }
                    #endif

                    )"_seg.replace(name, member.prettyName.capitalize(), member.prettyName, member.name, size.name, member.prettyType, size.type);
                } else {
                    enhancedParams + member.type + " " + member.name + "_";
                    enhancedInitializer + member.name + " { " + member.name + "_ }";
                }
                enhancedParams + defaultAssignment;
            }

            // Generate comparison and field declarations.
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

        Segment constructors = R"(
        $1 $0(
            $2
        ) VULKAN_HPP_NOEXCEPT
          : $3 {}

        $1 $0($0 const &) VULKAN_HPP_NOEXCEPT = default;
        $0(Vma$0 const & rhs) VULKAN_HPP_NOEXCEPT : $0(*reinterpret_cast<$0 const *>(&rhs)) {}
        )"_seg.replace(name, constexprToken, constructorParams, constructorInitializer);
        if (hasEnhancedConstructor) {
            constructors + R"(

            #if !defined( VULKAN_HPP_DISABLE_ENHANCED_MODE )
            $0(
                $1
            ) VULKAN_HPP_NOEXCEPT
              : $2 {}
            #endif
            )"_seg.replace(name, enhancedParams, enhancedInitializer);
        }

        content + nn + navigate(match) + R"(
        struct $0 {
          using NativeType = Vma$0;

        #if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
          $1

          $0& operator=($0 const &) VULKAN_HPP_NOEXCEPT = default;
        #endif

          $0& operator=(Vma$0 const & rhs) VULKAN_HPP_NOEXCEPT {
            *this = *reinterpret_cast<VMA_HPP_NAMESPACE::$0 const *>(&rhs);
            return *this;
          }

        #if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
          $2
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
          std::tuple<$3>
          reflect() const VULKAN_HPP_NOEXCEPT {
            return std::tie($4);
          }
        #endif

        $5

        public:
          $6
        };
        )"_seg.replace(name, constructors, setters, reflectTypes, reflectTie, comparison, fields);
    }

    R"(
    namespace VMA_HPP_NAMESPACE {
      $0
    }
    )"_seg.replace(content + navigate.reset).resolve(source.tree).generateHpp("structs");
    return structs;
}

std::tuple<Symbols, Symbols, Symbols> generateHandles(const Source& source, const Symbols& structs) {
    Symbols handles, uniqueHandles, functions;

    struct Handle : Symbol {
        Segment methodDecl, methodDef;
        Handle* owner = nullptr;
        Token destructor;
        explicit Handle(const Symbol& symbol) : Symbol(symbol) {
            static const std::unordered_map<std::string_view, std::string_view> handleDestructors {
                { "", /*    Namespace    */ "" },
                { "Allocator",              "destroy" },
                { "Pool",                   "destroyPool" },
                { "Allocation",             "freeMemory" },
                { "DefragmentationContext", "" },
                { "VirtualAllocation",      "virtualFree" },
                { "VirtualBlock",           "destroy" },
            };
            if (const auto& d = handleDestructors.find(*name); d != handleDestructors.end()) destructor = d->second;
            else throw std::runtime_error("No destructor for handle: " + std::string(*name));
        }
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
        if (handleVector.back().destructor) uniqueHandles.emplace_back(handles.back());
    }

    // Iterate VMA functions.
    const std::regex funcPattern { R"(VMA_CALL_PRE\s+(.+)\s+VMA_CALL_POST\s+(vma\w+)\s*(\([\s\S]+?\)\s*;))" };
    for (const auto& match : iterate(source, funcPattern)) {
        struct Param : Var {
            explicit Param(Var&& v) : Var(std::move(v)) {}
            enum class Type {
                INPUT, OUTPUT, OPTIONAL_OUTPUT
            } paramType = Type::INPUT;
        };
        std::vector<Param> params;
        for (auto& v : Var::parse(group(source, match, 3), match.position(3))) params.emplace_back(std::move(v));

        // Find handle.
        Handle* handle = params.empty() ? nullptr : findHandle(params[0]);
        Iterable methodParams(params.begin() + !!handle, params.end());
        if (handle && (params[0].constant || params[0].pointers))
            throw std::runtime_error("Unexpected handle parameter: " + std::string(params[0].type));

        // Convert name.
        Token originalName = group(source, match, 2);       // E.g. vmaCreateAllocator
        Token name = originalName.substr(3).capitalize(false); // E.g. createAllocator
        if (handle && startsWith(*name, "destroy") && (*name).substr(7) == *handle->name)
            name = "destroy"; // E.g. Allocator::destroyAllocator -> Allocator::destroy

        if (!handle) functions.emplace_back(name, match);

        // Find output params.
        std::vector<Param*> outputs; // Mandatory only.
        const Param* defaultOutputsFrom = nullptr;
        for (auto& param : methodParams) {
            // Ignore void* and char* - those are not outputs.
            if (!param.constant && param.pointers && (param.pointers > 1 || (param.kind != Var::Kind::VOID && param.kind != Var::Kind::CHAR))) {
                if (param.tag == Var::Tag::NOT_NULL) {
                    outputs.push_back(&param);
                    param.paramType = Param::Type::OUTPUT;
                } else {
                    param.paramType = Param::Type::OPTIONAL_OUTPUT;
                    if (defaultOutputsFrom > &param) defaultOutputsFrom = &param;
                    continue;
                }
            }
            defaultOutputsFrom = params.data() + params.size();
        }

        // Convert return type.
        auto returnType = [&](const Param& param) -> std::pair<Token, Token> {
            Token result = param.prettyType, unique;
            if (auto* h = findHandle(param); h && h->destructor) {
                std::string u { *result };
                u.insert(u.rfind(':') + 1 /*npos overflows to 0*/, "Unique");
                unique = u;
                if (h) {
                    if (!h->owner || h->owner == handle) h->owner = handle;
                    else throw std::runtime_error("Handle owner conflict: " + std::string(h->name));
                }
            }
            else if (*param.underlyingType == "VkBuffer") unique = "UniqueBuffer";
            else if (*param.underlyingType == "VkImage") unique = "UniqueImage";
            return { result, unique };
        };
        auto originalReturnType = group(source, match, 1);
        Token simpleReturn, vecAllocName;
        Segment enhancedReturn, uniqueReturn;
        if (outputs.empty() && originalReturnType == "VkBool32") enhancedReturn = simpleReturn = "VULKAN_HPP_NAMESPACE::Bool32";
        else if (originalReturnType == "VkResult") simpleReturn = "VULKAN_HPP_NAMESPACE::Result";
        else if (originalReturnType == "void") simpleReturn = "void";
        if (outputs.size() == 2 && !outputs[0]->lenIfNotNull && !outputs[1]->lenIfNotNull) { // Pair.
            auto a = returnType(*outputs[0]);
            auto b = returnType(*outputs[1]);
            (enhancedReturn, uniqueReturn) + "std::pair<" + b + ", " + a + ">"; // Pair order is reversed.
            if (!a.second && !b.second) uniqueReturn = {};
        } else if (outputs.size() == 1) {
            auto t = returnType(*outputs[0]);
            if (outputs[0]->lenIfNotNull) { // Vector.
                vecAllocName = std::string(*t.first) + "Allocator";
                (enhancedReturn, uniqueReturn) + "std::vector<" + t + ", " + vecAllocName + ">";
                if (!t.second) uniqueReturn = {};
            } else (enhancedReturn, uniqueReturn) + t; // Single value.
        }
        if (!simpleReturn || (enhancedReturn.blank() && !outputs.empty()))
            throw std::runtime_error("Unexpected output configuration for " + std::string(originalName));

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
        auto declarations = (simpleDecl, enhancedDecl, uniqueDecl, enhancedVecAllocDecl, uniqueVecAllocDecl);
        auto definitions = (simpleDef, enhancedDef, uniqueDef, enhancedVecAllocDef, uniqueVecAllocDef);
        auto simple = (simpleDecl, simpleDef);
        auto enhanced = ((enhancedDecl, enhancedDef), (uniqueDecl, uniqueDef),
                         (enhancedVecAllocDecl, enhancedVecAllocDef), (uniqueVecAllocDecl, uniqueVecAllocDef));
        auto enhancedDefinitions = ((enhancedDef, enhancedVecAllocDef), (uniqueDef, uniqueVecAllocDef));

        // Generate template for vector allocator.
        if (vecAllocName) {
            auto t = returnType(*outputs[0]);
            enhanced + "template <typename " + vecAllocName;
            (enhancedDecl, uniqueDecl) + " = std::allocator<" + t + ">";
            ((enhancedVecAllocDecl, enhancedVecAllocDef), (uniqueVecAllocDecl, uniqueVecAllocDef)) + "," + n +
                "          typename std::enable_if<std::is_same<typename " + vecAllocName + "::value_type, " + t + ">::value, int>::type";
            (enhancedVecAllocDecl, uniqueVecAllocDecl) + " = 0";
            enhanced + ">" + n;
        }

        // Generate macros.
        if (enhancedReturn.blank()) {
            enhancedReturn + "void";
            if (originalReturnType == "VkResult") enhanced + "VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS ";
        } else enhanced + "VULKAN_HPP_NODISCARD ";
        if (*simpleReturn != "void") simple + "VULKAN_HPP_NODISCARD ";
        definitions + "VULKAN_HPP_INLINE ";

        // Generate return type.
        if (originalReturnType == "VkResult") enhanced + "typename VULKAN_HPP_NAMESPACE::ResultValueType<";
        enhanced + *(enhancedReturn, uniqueReturn, enhancedReturn, uniqueReturn);
        simple + simpleReturn;
        if (originalReturnType == "VkResult") enhanced + ">::type";

        // Generate name.
        all + " ";
        if (handle) definitions + handle->name + "::";
        all + name;
        (uniqueDecl, uniqueDef, uniqueVecAllocDecl, uniqueVecAllocDef) + "Unique";

        // Generate parameter list.
        bool signatureTransformed = false;
        Segment simpleParams, enhancedParamsDecl, enhancedParamsDef;
        for (const auto& param : methodParams) {
            simpleParams + param.type + " " + param.name + "," + n;
            if (param.paramType == Param::Type::OUTPUT || !param.deducedArrays.empty()) { // Skip outputs and deduced array sizes.
                signatureTransformed = true;
                continue;
            }
            // Enhanced methods only transform pointer parameters.
            Segment type;
            if (param.pointers && param.kind != Var::Kind::VOID) {
                if (param.lenIfNotNull && !param.constant)
                    type + "VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<" + param.prettyType + "> const &";
                else if (param.lenIfNotNull && param.constant)
                    type + "VULKAN_HPP_NAMESPACE::ArrayProxy<" + param.prettyType + "> const &";
                else if (param.tag == Var::Tag::NOT_NULL)
                    type + param.prettyType + "&";
                else if (!param.constant && param.kind != Var::Kind::OTHER && param.kind != Var::Kind::CHAR)
                    type + "VULKAN_HPP_NAMESPACE::Optional<" + param.prettyType + ">";
            }
            if (type.blank()) type = param.type;
            else signatureTransformed = true;
            (enhancedParamsDecl, enhancedParamsDef) + type + " " + param.prettyName;
            if (&param >= defaultOutputsFrom) enhancedParamsDecl + (param.pointers ?
                        " VULKAN_HPP_DEFAULT_ARGUMENT_NULLPTR_ASSIGNMENT" : " VULKAN_HPP_DEFAULT_ARGUMENT_ASSIGNMENT");
            (enhancedParamsDecl, enhancedParamsDef) + "," + n;
        }
        Segment vecAllocParams = enhancedParamsDef;
        vecAllocParams + vecAllocName + "& " + vecAllocName.capitalize(false);
        (simpleParams, enhancedParamsDecl, enhancedParamsDef).pop().pop();
        all + "(";
        simple + simpleParams;
        (enhancedDecl, uniqueDecl) + enhancedParamsDecl;
        (enhancedDef, uniqueDef) + enhancedParamsDef;
        (enhancedVecAllocDecl, enhancedVecAllocDef, uniqueVecAllocDecl, uniqueVecAllocDef) + vecAllocParams;
        all + ")";
        if (handle) all + " const";
        declarations + ";";
        definitions + " {";

        // Generate deduction statements.
        for (const auto& param : methodParams) {
            if (param.deducedArrays.empty()) continue;
            Segment statement, assertChecks, exceptionChecks;
            statement + param.type + " " + param.prettyName + " = ";
            for (int i : param.deducedArrays) {
                const auto& t = params[i];
                if (t.paramType == Param::Type::OUTPUT) continue;
                statement + t.prettyName + ".size()" + " | ";
                assertChecks + n + "VULKAN_HPP_ASSERT(";
                exceptionChecks + n + "if (";
                if (t.tag != Var::Tag::NOT_NULL) {
                    assertChecks + t.prettyName + ".empty() || ";
                    exceptionChecks + "!" + t.prettyName + ".empty() && ";
                }
                assertChecks + t.prettyName + ".size() == " + param.prettyName + ");";
                exceptionChecks + t.prettyName + ".size() != " + param.prettyName +
                    ") throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING \"::";
                if (handle) exceptionChecks + handle->name + "::";
                exceptionChecks + name + ": " + t.prettyName + ".size() != " + param.prettyName + "\");";
            }
            enhancedDefinitions + n + "  " + statement.pop() + ";";
            if (param.deducedArrays.size() > 1) // No need to cross-check array sizes when there is only one.
                enhancedDefinitions + n + R"(
                #ifdef VULKAN_HPP_NO_EXCEPTIONS
                  $0
                #else
                  $1
                #endif
                )"_seg.replace(assertChecks, exceptionChecks);
        }

        // Generate output variable declarations.
        Token outputName;
        if (outputs.size() == 2 && !outputs[0]->lenIfNotNull && !outputs[1]->lenIfNotNull) { // Pair.
            outputName = "pair";
            enhancedDefinitions + n + "  " + enhancedReturn + " pair;" + // Pair order is reversed.
                n + "  " + outputs[0]->prettyType + "& " + outputs[0]->prettyName + " = pair.second;" +
                n + "  " + outputs[1]->prettyType + "& " + outputs[1]->prettyName + " = pair.first;";
        } else if (outputs.size() == 1) {
            outputName = outputs[0]->prettyName;
            enhancedDefinitions + n + "  ";
            if (outputs[0]->lenIfNotNull) { // Vector.
                // Unique variants should use a default allocator for the intermediate vector.
                enhancedDefinitions + std::tuple(enhancedReturn, "std::vector<$0>"_seg.replace(outputs[0]->prettyType)) + " " + outputName;
                Token size;
                if (outputs[0]->deducedArraySize == -1) { // Custom size deduction logic.
                    if (*originalName == "vmaGetHeapBudgets" && *outputs[0]->lenIfNotNull == "\"VkPhysicalDeviceMemoryProperties::memoryHeapCount\"")
                        size = "getMemoryProperties()->memoryHeapCount";
                    else throw std::runtime_error("Could not deduce output vector size for " + std::string(name));
                } else size = params[outputs[0]->deducedArraySize].prettyName;
                enhancedDefinitions + "(" + size;
                enhancedVecAllocDef + ", " + vecAllocName.capitalize(false); // Only for non-unique variant.
                enhancedDefinitions + ")";
            } else enhancedDefinitions + enhancedReturn + " " + outputName;
            enhancedDefinitions + ";";
        } else if (outputs.empty()) outputName = "result";
        else throw std::runtime_error("Unexpected output configuration for " + std::string(originalName));

        // Generate a call.
        Segment simpleCall, enhancedCall;
        (simpleCall, enhancedCall) + originalName + "(";
        if (handle) (simpleCall, enhancedCall) + "m_" + handle->name.capitalize(false) + ", ";
        for (const auto& param : methodParams) {
            Segment pass;
            if (param.pointers > 1 || param.kind != Var::Kind::VOID) {
                if (param.pointers && param.lenIfNotNull && param.kind != Var::Kind::VOID) // Array.
                    pass + param.prettyName + ".data()";
                else if (param.pointers && param.tag == Var::Tag::NOT_NULL) // Reference or local.
                    pass + "&" + param.prettyName;
                else if (param.pointers && !param.constant && param.kind != Var::Kind::OTHER && param.kind != Var::Kind::CHAR) // Optional.
                    pass + "static_cast<" + param.type + ">(" + param.prettyName + ")";
            }
            if (pass.blank()) pass + param.prettyName;
            if (param.kind == Var::Kind::VK || param.kind == Var::Kind::VMA) (simpleCall, enhancedCall) + // Cast Vk & Vma types.
                (param.pointers ? "reinterpret_cast<" : "static_cast<") + param.originalType + ">(";
            simpleCall + param.name;
            enhancedCall + std::move(pass);
            if (param.kind == Var::Kind::VK || param.kind == Var::Kind::VMA) (simpleCall, enhancedCall) + ")"; // Cast Vk & Vma types.
            (simpleCall, enhancedCall) + ", ";
        }
        (simpleCall, enhancedCall).pop() + ")";
        if (*simpleReturn != "void")
            simpleReturn + (" result = static_cast<" + (simpleReturn + (">( " + (simpleCall, enhancedCall) + " )")));
        simpleDef + n + "  " + std::move(simpleCall) + ";";
        enhancedDefinitions + n + "  " + enhancedCall + ";";

        // Generate result check.
        if (originalReturnType == "VkResult") { // TODO success codes!
            enhancedDefinitions + n + "  VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING \"::";
            if (handle) enhancedDefinitions + handle->name + "::";
            enhancedDefinitions + name + "\");";
        }

        // Generate unique conversion.
        (uniqueDef, uniqueVecAllocDef) + n + "  " + uniqueReturn + " " + outputName + "Unique";
        if (outputs.size() == 2 && !outputs[0]->lenIfNotNull && !outputs[1]->lenIfNotNull) { // Pair.
            uniqueDef + " {";
            for (auto* param : Iterable(outputs.rbegin(), outputs.rend())) {
                auto [type, unique] = returnType(*param);
                uniqueDef + n + "    ";
                if (unique) uniqueDef + unique + "(" + param->prettyName + (handle ? ", *this" : ", {}") + ")";
                else uniqueDef + param->prettyName;
                uniqueDef + ",";
            }
            uniqueDef.pop() + n + "  };";
        } else if (outputs.size() == 1) {
            if (outputs[0]->lenIfNotNull) { // Vector.
                uniqueVecAllocDef + "(" + vecAllocName.capitalize(false) + ")";
                (uniqueDef, uniqueVecAllocDef) + ";" + n + "  " + outputName + "Unique.reserve(" + outputName + ".size());" +
                    n + "  for (auto const& t : " + outputName + ") " + outputName + "Unique.emplace_back(t" + (handle ? ", *this" : ", {}") + ");";
            } else uniqueDef + " { " + outputs[0]->prettyName + (handle ? ", *this" : ", {}") + " };"; // Single value.
        } else if (!outputs.empty())
            throw std::runtime_error("Unexpected output configuration for " + std::string(originalName));

        // Generate return statement.
        if (*simpleReturn != "void") simpleDef + n + "  return result;";
        if (*simpleReturn != "void" || !outputs.empty()) {
            enhancedDefinitions + n + "  return ";
            if (originalReturnType == "VkResult")
                enhancedDefinitions + "VULKAN_HPP_NAMESPACE::detail::createResultValueType(result" + (outputs.empty() ? "" : ", ");
            if (!outputs.empty() || originalReturnType != "VkResult") {
                (uniqueDef, uniqueVecAllocDef) + "std::move(";
                enhancedDefinitions + outputName;
                (uniqueDef, uniqueVecAllocDef) + "Unique)";
            }
            if (originalReturnType == "VkResult") enhancedDefinitions + ")";
            enhancedDefinitions + ";";
        }
        definitions + n + "}";

        // Append methods to the handle
        bool hasUniqueVariant = !uniqueReturn.blank();
        auto appendMethods = [&](Segment& dst, const auto& variants) {
            auto& [simple, enhanced, unique, enhancedVecAlloc, uniqueVecAlloc] = variants;
            dst + nn + navigate(match) + "#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE"_seg + n + std::move(enhanced);
            if (vecAllocName) dst + n + std::move(enhancedVecAlloc);
            if (hasUniqueVariant) {
                dst + n + "#ifndef VULKAN_HPP_NO_SMART_HANDLE"_seg + n + std::move(unique);
                if (vecAllocName) dst + n + std::move(uniqueVecAlloc);
                dst + n + "#endif"_seg;
            }
            dst + n + (signatureTransformed ? "#endif"_seg : "#else"_seg) + n + std::move(simple);
            if (!signatureTransformed) dst + n + "#endif"_seg;
        };
        Handle& h = handle ? *handle : namespaceHandle;
        appendMethods(h.methodDecl, *declarations);
        appendMethods(h.methodDef, *definitions);
    }

    // Generate handle declarations.
    Segment::Vector<7> content;
    auto& [forwardDecls, uniqueDecls, declarations, handleTraits, cppTypeTraits, uniqueTraits, definitions] = *content;
    for (const Symbol& s : structs) forwardDecls + n + navigate(s) + "struct " + s.name + ";";
    for (Handle& h : handleVector) {
        (forwardDecls, handleTraits, cppTypeTraits) + n + navigate(h);
        forwardDecls + "class " + h.name + ";";
        handleTraits + R"(
        template <> struct isVulkanHandleType<VMA_HPP_NAMESPACE::$0> {
          static VULKAN_HPP_CONST_OR_CONSTEXPR bool value = true, vma = true;
        };
        )"_seg.replace(h.name);
        cppTypeTraits + R"(
        template <> struct CppType<Vma$0, VK_NULL_HANDLE> {
          using Type = VMA_HPP_NAMESPACE::$0;
        };
        )"_seg.replace(h.name);
        if (h.destructor) {
            Segment owner, destructor;
            if (h.owner) {
                owner + "VMA_HPP_NAMESPACE::" + h.owner->name;
                destructor + "getOwner()." + h.destructor + "(t)";
            } else {
                owner + "void";
                destructor + "t." + h.destructor + "()";
            }
            uniqueDecls + n + navigate(h) + R"(
            using Unique$0 = VULKAN_HPP_NAMESPACE::UniqueHandle<$0, detail::Dispatcher>;
            )"_seg.replace(h.name);
            uniqueTraits + n + navigate(h) + R"(
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
        declarations + nn + navigate(h) + R"(
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

          $2

        private:
          Vma$0 m_$1 = {};
        };
        )"_seg.replace(h.name, h.name.capitalize(false), h.methodDecl + navigate(h));
        definitions + nn + std::move(h.methodDef);
    }
    declarations + nn + std::move(namespaceHandle.methodDecl);
    definitions + nn + std::move(namespaceHandle.methodDef);
    content + navigate.reset;

    // Generate operators, some in vk:: are not found by ADL.
    Segment opTemplate = R"(
    template <typename T, typename std::enable_if<VULKAN_HPP_NAMESPACE::isVulkanHandleType<T>::vma, int>::type = 0>
    bool operator$0($1) {
      return $2;
    }
    )"_seg;
    Segment operators;
    operators + Segment(opTemplate).replace("==", "T const & v, std::nullptr_t", "!v");
    operators + Segment(opTemplate).replace("==", "std::nullptr_t, T const & v", "!v");
    operators + Segment(opTemplate).replace("!=", "T const & v, std::nullptr_t", "!!v");
    operators + Segment(opTemplate).replace("!=", "std::nullptr_t, T const & v", "!!v");
    opTemplate.replace("$0"_seg, "T const & lhs, T const & rhs",
        "static_cast<typename T::NativeType>(lhs) $0 static_cast<typename T::NativeType>(rhs)"_seg);
    for (const auto& op : { "<", "<=", ">", ">=", "==", "!=" }) operators + Segment(opTemplate).replace(op);

    R"(
    namespace VMA_HPP_NAMESPACE {
      $0

      namespace detail { class Dispatcher; } // VMA dispatcher is a no-op.
      using UniqueBuffer = VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Buffer, detail::Dispatcher>;
      using UniqueImage = VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Image, detail::Dispatcher>;
      $1

      $2

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

    namespace VMA_HPP_NAMESPACE {
      $6
    }
    )"_seg.replace(forwardDecls, uniqueDecls, declarations, handleTraits, cppTypeTraits, uniqueTraits, operators)
        .resolve(source.tree).generateHpp("handles");
    R"(
    namespace VMA_HPP_NAMESPACE {
      $0
    }
    )"_seg.replace(definitions).resolve(source.tree).generateHpp("funcs");
    return { handles, uniqueHandles, functions };
}

void generateStaticAssertions(const ConditionalTree& tree, const Symbols& structs, const Symbols& handles) {
    Segment content;
    for (const Symbol& s : structs) {
        content + nn + navigate(s) + R"(
        VULKAN_HPP_STATIC_ASSERT(sizeof(VMA_HPP_NAMESPACE::$0) == sizeof(Vma$0), "struct and wrapper have different size!");
        VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<VMA_HPP_NAMESPACE::$0>::value, "struct wrapper is not a standard layout!");
        VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<VMA_HPP_NAMESPACE::$0>::value, "$0 is not nothrow_move_constructible!");
        )"_seg.replace(s.name);
    }
    for (const Symbol& h : handles) {
        content + nn + navigate(h) + R"(
        VULKAN_HPP_STATIC_ASSERT(sizeof(VMA_HPP_NAMESPACE::$0) == sizeof(Vma$0), "handle and wrapper have different size!");
        VULKAN_HPP_STATIC_ASSERT(std::is_copy_constructible<VMA_HPP_NAMESPACE::$0>::value, "$0 is not copy_constructible!");
        VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<VMA_HPP_NAMESPACE::$0>::value, "$0 is not nothrow_move_constructible!");
        )"_seg.replace(h.name);
    }
    ("#include <vk_mem_alloc.hpp>" + content + navigate.reset).resolve(tree).generateHpp("static_assertions");
}

void generateModule(const ConditionalTree& tree, const Symbols& enums, const Symbols& structs,
                    const Symbols& handles, const Symbols& uniqueHandles, const Symbols& functions) {
    Segment exports, vmaPrivate, vkPrivate;

    // Generate export statements.
    for (const auto* list : { &enums, &structs, &handles, &uniqueHandles, &functions })
        for (const Symbol& t : *list)
            exports + n + navigate(t) + "using VMA_HPP_NAMESPACE::" + (list == &uniqueHandles ? "Unique" : "") + t.name + ";";

    // Some workarounds for compilation errors on MSVC...

    // fatal error C1116: unrecoverable error importing module 'vk_mem_alloc_hpp'.  Specialization of 'vma::operator ==' with arguments 'vma::Pool, 0'
    for (const Symbol& t : handles)
        vmaPrivate + n + navigate(t) + R"(
        template bool operator== <$0, 0>($0 const &, std::nullptr_t);
        template bool operator== <$0, 0>(std::nullptr_t, $0 const &);
        template bool operator!= <$0, 0>($0 const &, std::nullptr_t);
        template bool operator!= <$0, 0>(std::nullptr_t, $0 const &);
        template bool operator<  <$0, 0>($0 const &, $0 const &);
        template bool operator<= <$0, 0>($0 const &, $0 const &);
        template bool operator>  <$0, 0>($0 const &, $0 const &);
        template bool operator>= <$0, 0>($0 const &, $0 const &);
        template bool operator== <$0, 0>($0 const &, $0 const &);
        template bool operator!= <$0, 0>($0 const &, $0 const &);
        )"_seg.replace(t.name);

    // error C2678: binary '|': no operator found which takes a left-hand operand of type 'const vma::AllocationCreateFlagBits' (or there is no acceptable conversion)
    for (const Symbol& t : enums)
        if (endsWith(*t.name, "FlagBits"))
            vkPrivate + n + navigate(t) + "template<> struct FlagTraits<VMA_HPP_NAMESPACE::" + t.name + ">;";

    // error C2027: use of undefined type 'vk::UniqueHandleTraits<Type,Dispatch>'
    for (const Symbol& t : uniqueHandles)
        vkPrivate + n + navigate(t) + "template<> class UniqueHandleTraits<VMA_HPP_NAMESPACE::" + t.name + ", VMA_HPP_NAMESPACE::detail::Dispatcher>;";

    (exports, vmaPrivate, vkPrivate) + navigate.reset;

    // Don't forget Buffer and Image.
    exports + n + "using VMA_HPP_NAMESPACE::UniqueBuffer;" +
              n + "using VMA_HPP_NAMESPACE::UniqueImage;";
    vkPrivate + n + "template<> class UniqueHandleTraits<Buffer, VMA_HPP_NAMESPACE::detail::Dispatcher>;"  +
                        n + "template<> class UniqueHandleTraits<Image, VMA_HPP_NAMESPACE::detail::Dispatcher>;";

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
      using VMA_HPP_NAMESPACE::operator<;
      using VMA_HPP_NAMESPACE::operator<=;
      using VMA_HPP_NAMESPACE::operator>;
      using VMA_HPP_NAMESPACE::operator>=;
      using VMA_HPP_NAMESPACE::operator==;
      using VMA_HPP_NAMESPACE::operator!=;
      $0
    }

    module : private;
    // This is needed for template specializations to be visible outside the module when importing vulkan_hpp (is this a MSVC bug?).
    namespace VMA_HPP_NAMESPACE {
      $1
    }
    namespace VULKAN_HPP_NAMESPACE {
      $2
    }
    )"_seg.replace(exports, vmaPrivate, vkPrivate).resolve(tree).generate("vk_mem_alloc.cppm");
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
        const auto [handles, uniqueHandles, functions] = generateHandles(source, structs);
        generateStaticAssertions(source.tree, structs, handles);
        generateModule(source.tree, enums, structs, handles, uniqueHandles, functions);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}