#include "json.h"

using namespace std;

namespace json {

    namespace {

        // ---------- Node Loaders ----------

        Node LoadNode(istream &input);

        Node LoadNullOrBool(std::istream &input) {
            std::string keyword;
            while (isalpha(input.peek())) {
                keyword += input.get();
            }

            if (keyword == "null"s) {
                return Node(nullptr);
            } else if (keyword == "true"s) {
                return Node(true);
            } else if (keyword == "false"s) {
                return Node(false);
            } else {
                throw ParsingError("Invalid nullOrBool value");
            }
        }

        Node LoadNumber(istream &input) {
            using namespace std::literals;

            string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(stoi(parsed_num));
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(stod(parsed_num));
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream &input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        Node LoadArray(istream &input) {
            Array result;

            for (char c; input >> c;) {
                if (c == ']') {
                    return Node(move(result));
                }

                if (c != ',') {
                    input.putback(c);
                }

                result.push_back(LoadNode(input));
            }
            throw ParsingError("Unexpected end of Array");
        }

        Node LoadDict(istream &input) {
            Dict result;

            for (char c; input >> c;) {
                if (c == '}') {
                    return Node(move(result));
                }

                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }
            throw ParsingError("Unexpected end of Dict");
        }

        Node LoadNode(istream &input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (isalpha(c)) {
                input.putback(c);
                return LoadNullOrBool(input);
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    } // namespace

    // ---------- Node Constructors ----------

    Node::Node(Value value) : variant(std::move(value)) {}

    // ---------- Node TypeCheck ----------

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }

    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }

    bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
    }

    bool Node::IsString() const {
        return holds_alternative<string>(*this);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(*this);
    }

    // ---------- Node Getters ----------

    bool Node::AsBool() const {
        if (!IsBool())
            throw logic_error("Incorrect type");
        return get<bool>(*this);
    }

    int Node::AsInt() const {
        if (!IsInt())
            throw logic_error("Incorrect type");
        return get<int>(*this);
    }

    double Node::AsDouble() const {
        if (IsPureDouble())
            return get<double>(*this);
        if (IsInt())
            return static_cast<double>(get<int>(*this));
        throw logic_error("Incorrect type");
    }

    const string &Node::AsString() const {
        if (!IsString())
            throw logic_error("Incorrect type");
        return get<string>(*this);
    }

    const Array &Node::AsArray() const {
        if (!IsArray())
            throw logic_error("Incorrect type");
        return get<Array>(*this);
    }

    const Dict &Node::AsMap() const {
        if (!IsMap())
            throw logic_error("Incorrect type");
        return get<Dict>(*this);
    }

    // ---------- PrintContext ----------

    void PrintContext::PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext PrintContext::Indented() const {
        return {out, indent_step, indent_step + indent};
    }

    // ---------- Node Print ----------

    template <typename Value>
    void PrintValue(const Value &value, const PrintContext &context) {
        context.out << value;
    }

    void PrintValue(nullptr_t, const PrintContext &context) {
        context.out << "null";
    }

    void PrintValue(bool value, const PrintContext &context) {
        context.out << (value ? "true" : "false");
    }

    void PrintValue(const string &value, const PrintContext &context) {
        context.out << "\"";
        for (auto ch : value) {
            switch (ch) {
            case '\n':
                context.out << "\\n"sv;
                break;
            case '\t':
                context.out << "\\t"sv;
                break;
            case '\r':
                context.out << "\\r"sv;
                break;
            case '\"':
                context.out << "\\\""sv;
                break;
            case '\\':
                context.out << "\\\\"sv;
                break;
            default:
                context.out << ch;
            }
        }
        context.out << "\"";
    }

    void PrintValue(const Array &array, const PrintContext &ctx) {
        ctx.out << '[';
        bool first = true;
        PrintContext IndentedCtx = ctx.Indented();
        for (const auto &node : array) {
            if (!first) {
                ctx.out << ',';
            } else {
                first = false;
            }
            IndentedCtx.out << '\n';
            IndentedCtx.PrintIndent();
            PrintNode(node, IndentedCtx.out, IndentedCtx);
        }
        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << ']';
    }

    void PrintValue(const Dict &dict, const PrintContext &ctx) {
        ctx.out << '{';
        bool first = true;
        PrintContext newCtx = ctx.Indented();
        for (const auto &[key, value] : dict) {
            if (!first) {
                ctx.out << ',';
            } else {
                first = false;
            }
            ctx.out << '\n';
            newCtx.PrintIndent();
            PrintValue(key, ctx);
            ctx.out << ": ";
            PrintNode(value, ctx.out, newCtx);
        }
        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << '}';
    }

    void PrintNode(const Node &node, std::ostream &out, const PrintContext &ctx) {
        visit([&out, &ctx](const auto &value) { PrintValue(value, ctx); }, node.GetValue());
    }

    // ---------- Document ----------

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node &Document::GetRoot() const {
        return root_;
    }

    Document Load(istream &input) {
        return Document{LoadNode(input)};
    }

    void Print(const Document &doc, std::ostream &output) {
        PrintContext context{output}; // Создаем контекст вывода
        PrintNode(doc.GetRoot(), output, context);
    }

} // namespace json