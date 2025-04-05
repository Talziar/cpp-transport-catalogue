#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Number = std::variant<int, double>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };
    struct PrintContext;

    class Node final
        : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        // Делаем доступными все конструкторы родительского класса variant
        using variant::variant;

        using Value = variant;

        Node(Value value);

        bool IsNull() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsPureDouble() const;
        bool IsDouble() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;

        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string &AsString() const;
        const Array &AsArray() const;
        const Dict &AsMap() const;

        const Value &GetValue() const {
            return *this;
        }

        Value &GetValue() {
            return *this;
        }

        bool operator==(const Node &other) const { return this->GetValue() == other.GetValue(); }
        bool operator!=(const Node &other) const { return !(*this == other); }
    };

    struct PrintContext {
        std::ostream &out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const;
        PrintContext Indented() const;
    };

    void PrintNode(const Node &node, std::ostream &out, const PrintContext &ctx);

    class Document {
    public:
        explicit Document(Node root);

        const Node &GetRoot() const;

        bool operator==(const Document &other) const { return root_ == other.root_; }
        bool operator!=(const Document &other) const { return !(*this == other); }

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    void Print(const Document &doc, std::ostream &output);

} // namespace json