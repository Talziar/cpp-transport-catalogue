#pragma once

#include "json.h"

namespace json {

    class BaseContext;
    class DictItemContext;
    class DictValueContext;
    class ArrayValueContext;
    class NonValueContext;

    class Builder {
    public:
        Builder();

        Node Build();

        DictValueContext Key(std::string key);

        NonValueContext Value(Node::Value value);

        DictItemContext StartDict();

        BaseContext EndDict();

        ArrayValueContext StartArray();

        BaseContext EndArray();

        

    private:
        Node root_;
        std::vector<Node *> nodes_stack_;

        Node::Value &GetCurrentValue();
        const Node::Value &GetCurrentValue() const;

        void AssertNewObjectContext() const;
        void AddObject(Node::Value value, bool one_shot);
    };

    class BaseContext {
    public:
        BaseContext(Builder &builder);
        Node Build();
        DictValueContext Key(std::string key);
        NonValueContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayValueContext StartArray();
        BaseContext EndDict();
        BaseContext EndArray();
        Builder &GetBuilder();

    private:
        Builder &builder_;
    };

    class NonValueContext : private BaseContext {
    public:
        NonValueContext(BaseContext base) : BaseContext(base.GetBuilder()) {}
        using BaseContext::BaseContext;
        using BaseContext::Build;
        using BaseContext::EndArray;
        using BaseContext::EndDict;
        using BaseContext::GetBuilder;
        using BaseContext::StartArray;
        using BaseContext::StartDict;
    };

    class DictItemContext : private BaseContext {
    public:
        DictItemContext(BaseContext base) : BaseContext(base.GetBuilder()) {}
        using BaseContext::BaseContext;
        using BaseContext::EndDict;
        using BaseContext::GetBuilder;
        using BaseContext::Key;
    };

    class DictValueContext : private BaseContext {
    public:
        using BaseContext::BaseContext;
        using BaseContext::GetBuilder;
        using BaseContext::StartArray;
        using BaseContext::StartDict;
        DictValueContext(BaseContext base) : BaseContext(base.GetBuilder()) {}
        DictItemContext Value(Node::Value value) { return GetBuilder().Value(value).GetBuilder(); }
    };

    class ArrayValueContext : private BaseContext {
    public:
        ArrayValueContext(BaseContext base) : BaseContext(base.GetBuilder()) {}
        using BaseContext::BaseContext;
        using BaseContext::EndArray;
        using BaseContext::GetBuilder;
        using BaseContext::StartArray;
        using BaseContext::StartDict;

        ArrayValueContext Value(Node::Value value) {
            return GetBuilder().Value(value).GetBuilder();
        }
    };

} // namespace json
