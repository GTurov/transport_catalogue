#include "json_builder.h"

namespace json {

using namespace std::literals;

Node detail::ValueToNode(const Data &value) {
    return std::visit([](auto &value){return Node(value);},value);
}

DictValueContext Builder::Key(const std::string &key) {
    // Если режим правки и на вершине стека мап, добавляем ноду со именем ключа
    // Иначе выводим разные ошибки
    switch (state_) {
    case state::EMPTY:  throw std::logic_error("empty node key add attempt"s); break;
    case state::EDITION: {
        if (nodesStack_.top()->IsDict()) {
            nodesStack_.push(std::make_unique<Node>(key));
        } else {
            throw std::logic_error(IsDictKeyTop()?
                                       "dict key entered twice"s:
                                       "not dict node key add attempt"s);
        }
    } break;
    case state::READY: throw std::logic_error("ready node key add attempt"s); break;
    default: throw std::logic_error("dict key common error"s);
    }
    return DictValueContext(*this);
}

Builder& Builder::Value(const Data &value) {
    // Если состояние пустое, добавляем ноду и выставляем реди.
    // Если состояние правки и на вершине массив, добавляем в него ноду
    // Если состояние правки, размер стека >1 и на вершине строка(имя ключа), забираем строку и создаём пару
    // Если нода собрана, выводим ошибку
    switch (state_) {
    case state::EMPTY: {
        nodesStack_.push(std::make_unique<Node>(detail::ValueToNode(value)));
        state_ = state::READY;
    } break;
    case state::EDITION: {
        if (nodesStack_.top()->IsArray()) {
            json::Array tmp = std::move(nodesStack_.top()->AsArray());
            tmp.emplace_back(detail::ValueToNode(value));
            *nodesStack_.top() = Node(std::move(tmp));
        } else if (IsDictKeyTop()) {
            std::string key = std::move(nodesStack_.top()->AsString());
            nodesStack_.pop();
            json::Dict dict = std::move(nodesStack_.top()->AsDict());
            dict.insert({key,detail::ValueToNode(value)});
            *nodesStack_.top() = Node(std::move(dict));
        } else {
            throw std::logic_error("dict value without key add attempt"s);
        }
    } break;
    case state::READY: throw std::logic_error("ready node value add attempt"s); break;
    default: throw std::logic_error("value common error"s);
    }
    return *this;
}

DictItemContext Builder::StartDict() {
    // Если состояние пустое, выставляем правку и пушим ноду мапа в стек
    // Если правка, пушим стек
    // Если нода готова, ошибка
    switch (state_) {
    case state::EMPTY: state_ = state::EDITION; nodesStack_.push(std::make_unique<Node>(Dict())); break;
    case state::EDITION:
        if (!nodesStack_.top()->IsDict()) {
            nodesStack_.push(std::make_unique<Node>(Dict()));
        } else {
            throw std::logic_error("start dict in another dict error"s);
        } break;
    case state::READY: throw std::logic_error("ready node start dict attempt"s); break;
    default: throw std::logic_error("start dict common error"s);
    }
    return DictItemContext(*this);
}

Builder& Builder::EndDict() {
    // Если состояние пустое, то ошибка
    // Если решим правки, проверяем, что на вершине стека мап
    // Если мап единственный в стеке, значит нода собрана
    // Если в стеке ещё что-то есть, убираем мап из стека и вызываем вставку значения
    // Если нода собрана, ошибка
    switch (state_) {
    case state::EMPTY: throw std::logic_error("empty node end dict attempt"s); break;
    case state::EDITION: {
        if (nodesStack_.top()->IsDict()) {
            if (nodesStack_.size() == 1) {
                state_ = state::READY;
            } else {
                json::Dict value = std::move(nodesStack_.top()->AsDict());
                nodesStack_.pop();
                Value(value);
            }
        } else {
            throw std::logic_error(nodesStack_.top()->IsString()?
                                       "dict value expected"s:
                                       "it is not a dict"s);
        }
    } break;
    case state::READY: throw std::logic_error("ready node end dict attempt"s); break;
    default: throw std::logic_error("end dict common error"s);
    }
    return *this;
}

ArrayItemContext Builder::StartArray() {
    // Если нода пустая, выставляем правку и пушим массив в стек
    // Если правка и наверху стека не пустой мап, пушим массив в стек
    // Если нода готова, ошибка
    switch (state_) {
    case state::EMPTY: state_ = state::EDITION; nodesStack_.push(std::make_unique<Node>(Array())); break;
    case state::EDITION: {
        if (nodesStack_.top()->IsDict()) {
            throw std::logic_error("start array error. enter a dict key first"s);
        }
        nodesStack_.push(std::make_unique<Node>(Array()));
    } break;
    case state::READY: throw std::logic_error("ready node start array attempt"s); break;
    default: throw std::logic_error("start array error"s);
    }
    return ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    // Если состояние пустое, выводим ошибку
    // Проверяем, что на вершне стека массив
    // Если в стеке только массив, значит выставляем состояние реди
    // Если в стеке что-то есть, изымаем ноду и вызываем Value()
    // Если состояние готовности, выводим ошибку
    switch (state_) {
    case state::EMPTY: throw std::logic_error("empty node end array attempt"s); break;
    case state::EDITION: {
        if (nodesStack_.top()->IsArray()) {
            if (nodesStack_.size() == 1) {
                state_ = state::READY;
            } else {
                json::Array value = std::move(nodesStack_.top()->AsArray());
                nodesStack_.pop();
                Value(value);
            }
        } else {
            throw std::logic_error("non-array node end array attempt"s);
        }
    } break;
    case state::READY: throw std::logic_error("ready node end array attempt"s); break;
    default: throw std::logic_error("end aray common error"s);
    }
    return *this;
}

Node Builder::Build() {
    if (state_ == state::READY) {
        return *nodesStack_.top();
    } else {
        throw std::logic_error("building of unready node"s);
    }
}

void Builder::Clear() {
    while (!nodesStack_.empty()) {
        nodesStack_.pop();
    }
    state_ = state::EMPTY;
}

DictItemContext ArrayItemContext::StartDict() {
    return b_.StartDict();
}

DictValueContext DictItemContext::Key(const std::string &key) {
    return b_.Key(key);
}

} // namespace json

