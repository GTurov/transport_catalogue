#pragma once

#include <memory>
#include <stack>
#include <string>

#include "json.h"


#include <iostream> //debug
using namespace std::literals; //debug

namespace json {

namespace detail {

Node valueToNode(const Data &value);

} //namespace detail

class DictValueContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    Builder() {}
    ~Builder() {clear();}
    DictValueContext key(const std::string& key);
    Builder& value(const Data& value);
    DictItemContext startDict();
    Builder& endDict();
    ArrayItemContext startArray();
    Builder& endArray();
    Node build();
    void clear();

private:
    bool isDictKeyTop() {return (nodesStack_.size()>1) && nodesStack_.top()->isString();}
private:
    enum class state {EMPTY, EDITION, READY};
    state state_ = state::EMPTY;
    std::stack<std::unique_ptr<Node>> nodesStack_;
};

class ArrayItemContext {
public:
    ArrayItemContext(Builder& b)
        :b_(b){}
    ArrayItemContext value(const Data& value) {return ArrayItemContext(b_.value(value));}
    DictItemContext startDict();
    ArrayItemContext startArray() {return b_.startArray();}
    Builder& endArray() {return b_.endArray();}
private:
    Builder& b_;
};

class DictItemContext {
public:
    DictItemContext(Builder& b)
        :b_(b){}
    DictValueContext key(const std::string& key);
    Builder& endDict() {return b_.endDict();}
private:
    Builder& b_;
};

class DictValueContext {
public:
    DictValueContext(Builder& b)
        :b_(b){}
    DictItemContext value(const Data& value) {return DictItemContext(b_.value(value));}
    DictItemContext startDict() {return b_.startDict();}
    ArrayItemContext startArray() {return b_.startArray();}
private:
    Builder& b_;
};



} //namespace json
