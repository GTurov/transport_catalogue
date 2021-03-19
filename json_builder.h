#pragma once

#include <memory>
#include <stack>
#include <string>

#include "json.h"


#include <iostream> //debug
using namespace std::literals; //debug

namespace json {


class DictValueContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    Builder() {}
    ~Builder() {Clear();}
    DictValueContext Key(const std::string& key);
    DictValueContext Key(std::string&& key);
    Builder& Value(const Data& value);
    Builder& Value(Data&& value);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    Node Build();
    void Clear();

private:
    bool IsDictKeyTop() {return (nodesStack_.size()>1) && nodesStack_.top()->IsString();}
private:
    enum class state {EMPTY, EDITION, READY};
    state state_ = state::EMPTY;
    std::stack<std::unique_ptr<Node>> nodesStack_;
};

class ArrayItemContext {
public:
    ArrayItemContext(Builder& b)
        :b_(b){}
    ArrayItemContext Value(const Data& value) {return ArrayItemContext(b_.Value(value));}
    ArrayItemContext Value(Data&& value) {return ArrayItemContext(b_.Value(value));}
    DictItemContext StartDict();
    ArrayItemContext StartArray() {return b_.StartArray();}
    Builder& EndArray() {return b_.EndArray();}
private:
    Builder& b_;
};

class DictItemContext {
public:
    DictItemContext(Builder& b)
        :b_(b){}
    DictValueContext Key(const std::string& key);
    DictValueContext Key(std::string&& key);
    Builder& EndDict() {return b_.EndDict();}
private:
    Builder& b_;
};

class DictValueContext {
public:
    DictValueContext(Builder& b)
        :b_(b){}
    DictItemContext Value(const Data& value) {return DictItemContext(b_.Value(value));}
    DictItemContext Value(Data&& value) {return DictItemContext(b_.Value(value));}
    DictItemContext StartDict() {return b_.StartDict();}
    ArrayItemContext StartArray() {return b_.StartArray();}
private:
    Builder& b_;
};



} //namespace json
