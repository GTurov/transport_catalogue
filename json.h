#pragma once

#include <iostream>
#include <initializer_list>
#include <map>
#include <string>
#include <variant>
#include <vector>


namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Data = std::variant<std::nullptr_t, bool, int, double, std::string, Array, Dict>;

std::ostream& operator<<(std::ostream& out, const Data& d);

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    Node() = default;
    Node(std::nullptr_t) {}
    Node(bool value);
    Node(int value);
    Node(double value);
    Node(std::string value);
    Node(Array array);
    Node(Dict map);

    bool asBool() const;
    int asInt() const;
    double asDouble() const;
    const std::string& asString() const;
    const Array& asArray() const;
    const Dict& asDict() const;

    bool isNull() const;
    bool isBool() const;
    bool isInt() const;
    bool isDouble() const;
    bool isPureDouble() const;
    bool isString() const;
    bool isArray() const;
    bool isDict() const;

    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

    Data content() const;

private:
    Data data_;
};

std::ostream& operator<<(std::ostream& out, const Node& d);

class Document {
public:
    explicit Document(Node root);

    const Node& getRoot() const;

    bool operator==(const Document& other) const {
        return root_ == other.root_;
    }
    bool operator!=(const Document& other) const {
        return root_ != other.root_;
    }

private:
    Node root_;
};

Document load(std::istream& input);

void print(const Document& doc, std::ostream& output);

}  // namespace json
