#include "json_builder.h"

using namespace json;
using namespace std::literals;

namespace json {

    // ---------- Builder----------

    Builder::Builder()
        : root_(), nodes_stack_{&root_} {}

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Attempt to build JSON which isn't finalized"s);
        }
        return std::move(root_);
    }

    DictValueContext Builder::Key(std::string key) {
        Node::Value &host_value = GetCurrentValue();

        if (!std::holds_alternative<Dict>(host_value)) {
            throw std::logic_error("Key() outside a dict"s);
        }

        nodes_stack_.push_back(
            &std::get<Dict>(host_value)[std::move(key)]);
        return *this;
    }

    NonValueContext Builder::Value(Node::Value value) {
        AddObject(std::move(value), /* one_shot */ true);
        return *this;
    }

    DictItemContext Builder::StartDict() {
        AddObject(Dict{}, /* one_shot */ false);
        return *this;
    }

    BaseContext Builder::EndDict() {
        if (!std::holds_alternative<Dict>(GetCurrentValue())) {
            throw std::logic_error("EndDict() outside a dict"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    ArrayValueContext Builder::StartArray() {
        AddObject(Array{}, /* one_shot */ false);
        return *this;
    }

    BaseContext Builder::EndArray() {
        if (!std::holds_alternative<Array>(GetCurrentValue())) {
            throw std::logic_error("EndDict() outside an array"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node::Value &Builder::GetCurrentValue() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Attempt to change finalized JSON"s);
        }
        return nodes_stack_.back()->GetValue();
    }

    const Node::Value &Builder::GetCurrentValue() const {
        return const_cast<Builder *>(this)->GetCurrentValue();
    }

    void Builder::AssertNewObjectContext() const {
        if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
            throw std::logic_error("New object in wrong context"s);
        }
    }

    void Builder::AddObject(Node::Value value, bool one_shot) {
        Node::Value &host_value = GetCurrentValue();
        if (std::holds_alternative<Array>(host_value)) {
            Node &node = std::get<Array>(host_value).emplace_back(std::move(value));
            if (!one_shot) {
                nodes_stack_.push_back(&node);
            }
        } else {
            AssertNewObjectContext();
            host_value = std::move(value);
            if (one_shot) {
                nodes_stack_.pop_back();
            }
        }
    }

    // ---------- BaseContext ----------

    BaseContext::BaseContext(Builder &builder) : builder_(builder) {}

    Node BaseContext::Build() { return builder_.Build(); }

    DictValueContext BaseContext::Key(std::string key) { return builder_.Key(key); }

    NonValueContext BaseContext::Value(Node::Value value) { return builder_.Value(value); }

    DictItemContext BaseContext::StartDict() { return builder_.StartDict(); }

    ArrayValueContext BaseContext::StartArray() { return builder_.StartArray(); }

    BaseContext BaseContext::EndDict() { return builder_.EndDict(); }

    BaseContext BaseContext::EndArray() { return builder_.EndArray(); }

    Builder &BaseContext::GetBuilder() { return builder_; }

} // namespace json
