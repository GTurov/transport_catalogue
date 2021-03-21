#include "json.h"

#include <array>
#include <iomanip>

using namespace std;

namespace json {

namespace {

Node loadNode(istream& input);

Node loadArray(istream& input) {
    Array result;

    for (char c; input >> c /*&& c != ']'*/;) {
        if (c == ']') {
            return Node(move(result));
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(loadNode(input));
    }
    //cout<<result;
    throw json::ParsingError("Array parsing error"s);
}

using Number = std::variant<int, double>;

Number loadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsedNum;

    // Считывает в parsedNum очередной символ из input
    auto readChar = [&parsedNum, &input] {
        parsedNum += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsedNum из input
    auto readDigits = [&input, readChar] {
        if (!std::isdigit(input.peek())) {
            //cerr<<(char)input.peek();
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            readChar();
        }
    };

    if (input.peek() == '-') {
        readChar();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        readChar();
        // После 0 в JSON не могут идти другие цифры
    } else {
        readDigits();
    }

    bool isInt = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        readChar();
        readDigits();
        isInt = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        readChar();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            readChar();
        }
        readDigits();
        isInt = false;
    }

    try {
        if (isInt) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsedNum);
            } catch (...) {
                // В случае неудачи, например, при переполнении
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsedNum);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsedNum + " to number"s);
    }
}

Node loadString(istream& input) {
    string line;
    int i = 0;
    for (char c; input >> std::noskipws >> c /*&& c != '"'*/;) {
        ++i;
        if (c == '"') {
            input>>std::skipws;
            return Node(move(line));
        }
        if (c == '\\') {
            input >> c;
            ++i;
            switch (c) {
            case '"': line.push_back('"'); break;
            case 'n': line.push_back('\n'); break;
            case 'r': line.push_back('\r'); break;
            case '\\': line.push_back('\\'); break;
            case 't': line.push_back('\t'); break;
            default:
                input>>std::skipws;
                throw json::ParsingError("bad string: symbol #"s+ to_string(i));
            }
        } else {
            line.push_back(c);
        }
    }
    throw json::ParsingError("String parsing error"s);
}

Node loadDict(istream& input) {
    Dict result;
    for (char c; input >> c /*&& c != '}'*/;) {
        if (c == '}') {
            return Node(move(result));
        }
        if (c == ',') {
            input >> c;
        }
        string key = loadString(input).asString();
        input >> c;
        result.insert({move(key), loadNode(input)});
    }
    throw json::ParsingError("Map parsing error"s);
}

Node loadBool(istream& input) {
    static const string_view trueSv = "true"sv;
    static const string_view falseSv = "false"sv;
    size_t charCount = 0;
    size_t trueCount = 0;
    size_t falseCount = 0;
    for (char c; input >> c;) {
        ++charCount;
        if (c == falseSv[charCount - 1]) {
            ++falseCount;
            if (falseCount == falseSv.size()) {
                if (iswalnum(input.peek())) {
                    throw ParsingError("Fail"s);
                }
                return Node(false);
            }
            continue;
        }
        if (c == trueSv[charCount - 1]) {
            ++trueCount;
            if (trueCount == trueSv.size()) {
                if (iswalnum(input.peek())) {
                    throw ParsingError("Fail"s);
                }
                return Node(true);
            }
            continue;
        }

        throw json::ParsingError("bool type error"s);
    }
    throw json::ParsingError("bool type error"s);
}

Node loadNull(istream& input) {
    static const string_view nullSv = "null"sv;
    size_t charCount = 0;
    size_t nullCount = 0;
    for (char c; input >> c;) {
        ++charCount;
        if (c == nullSv[charCount-1]) {
            ++nullCount;
            if (nullCount == nullSv.size()) {
                return Node(nullptr);
            }
            continue;
        }
        throw json::ParsingError("null type error"s);
    }
    throw json::ParsingError("null type error"s);
}

Node loadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return loadArray(input);
    } else if (c == '{') {
        return loadDict(input);
    } else if (c == '"') {
        return loadString(input);
    } else if (c == 't' || c == 'f'){
        input.putback(c);
        return loadBool(input);
    } else if (c == 'n') {
        input.putback(c);
        return loadNull(input);
    } else {
        input.putback(c);
        Number x = loadNumber(input);
        return (std::holds_alternative<int>(x)?Node(std::get<int>(x)):Node(std::get<double>(x)));
    }
}

}  // namespace

Node::Node(bool value)
    : data_(value) {
}

Node::Node(int value)
    : data_(value) {
}

Node::Node(double value)
    : data_(value) {
}

Node::Node(string value)
    : data_(move(value)) {
}

Node::Node(Array array)
    : data_(move(array)) {
}

Node::Node(Dict map)
    : data_(move(map)) {
}

bool Node::asBool() const {
    if (std::holds_alternative<bool>(data_)) {
        return get<bool>(data_);
    } else {
        throw std::logic_error(""s);
    }
}

int Node::asInt() const {
    if (std::holds_alternative<int>(data_)) {
        return get<int>(data_);
    } else {
        throw std::logic_error(""s);
    }
}

double Node::asDouble() const {
    if (std::holds_alternative<int>(data_)) {
        return get<int>(data_);
    } else if (std::holds_alternative<double>(data_)) {
            return get<double>(data_);
    } else {
        throw std::logic_error(""s);
    }
}

const string& Node::asString() const {
    if (std::holds_alternative<std::string>(data_)) {
        return get<std::string>(data_);
    } else {
        throw std::logic_error(""s);
    }
}

const Array& Node::asArray() const {
    if (std::holds_alternative<Array>(data_)) {
        return get<Array>(data_);
    } else {
        throw std::logic_error(""s);
    }
}

const Dict& Node::asDict() const {
    if (std::holds_alternative<Dict>(data_)) {
        return get<Dict>(data_);
    } else {
        throw std::logic_error(""s);
    }
}

bool Node::isNull() const {
    return (data_ == Data());
}

bool Node::isBool() const {
    return std::holds_alternative<bool>(data_);
}

bool Node::isInt() const {
    return std::holds_alternative<int>(data_);
}

bool Node::isDouble() const {
    return std::holds_alternative<int>(data_) || std::holds_alternative<double>(data_);
}

bool Node::isPureDouble() const {
    return std::holds_alternative<double>(data_);
}

bool Node::isString() const {
    return std::holds_alternative<std::string>(data_);
}

bool Node::isArray() const {
    return std::holds_alternative<Array>(data_);
}

bool Node::isDict() const {
    return std::holds_alternative<Dict>(data_);
}


bool Node::operator==(const Node &other) const {
    return data_ == other.data_;
}

bool Node::operator!=(const Node &other) const {
    return data_ != other.data_;
}

Data Node::content() const {
    return data_;
}



Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::getRoot() const {
    return root_;
}

Document load(istream& input) {
    return Document{loadNode(input)};
}

struct PrintContext {
    std::ostream& out;
    int indentStep = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indentStep, indentStep + indent};
    }
};

void printNode(const Node& value, const PrintContext& ctx);

template <typename Value>
void printValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void printString(const string& value, std::ostream& out) {
    out<<"\""sv;
    for (char c: value) {
        switch (c) {
        case '"': out<<"\\\""sv; break;
        case '\n': out<<"\\n"sv; break;
        case '\r': out<<"\\r"sv; break;
        case '\\': out<<"\\\\"sv; break;
        case '\t': out<<"\\t"sv; break;
        default:
            out<<c;
        }
    }
    out<<"\""sv;
}

template <>
void printValue<std::string>(const std::string& value, const PrintContext& ctx) {
    printString(value, ctx.out);
}

template <>
void printValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

template <>
void printValue<bool>(const bool& value, const PrintContext& ctx) {
    ctx.out << (value ? "true"sv : "false"sv);
}

template <>
void printValue<Array>(const Array& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "[\n"sv;
    bool first = true;
    auto innerCtx = ctx.Indented();
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        innerCtx.PrintIndent();
        printNode(node, innerCtx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}

template <>
void printValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto innerCtx = ctx.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        innerCtx.PrintIndent();
        printString(key, ctx.out);
        out << ": "sv;
        printNode(node, innerCtx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}

void printNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value) {
            printValue(value, ctx);
        },
        node.content());
}

std::ostream& operator<<(std::ostream& out, const Node& d) {
    printNode(d, PrintContext{out});
    return out;
}

void print(const Document& doc, std::ostream& output) {
    //visit(VariantPrinter{output}, doc.GetRoot().Content());
    printNode(doc.getRoot(), PrintContext{output});
}

}  // namespace json
