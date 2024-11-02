#include "json.h"

using namespace std;

namespace json {

    namespace
    {

        using Number = std::variant<int, double>;

        Node LoadNode(istream& input);

        Node LoadNull(std::istream& input)
        {
            std::string value;
            char c;
            while (input.get(c) && std::isalpha(c)) {
                value += c;
            }
            input.putback(c);  // Возвращаем символ в поток
            if (value != "null") {
                throw ParsingError("Invalid null value: " + value);
            }
            return Node(nullptr);
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
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
            }
            else {
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
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
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
                }
                else if (ch == '\\') {
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
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        bool LoadBool(std::istream& input)
        {
            std::string value;
            char c;
            while (input.get(c) && std::isalpha(c)) {
                value += c;
            }
            input.putback(c);  // Возвращаем символ в поток
            if (value == "true") return true;
            if (value == "false") return false;
            throw ParsingError("Invalid boolean value: " + value);
        }

        Node LoadArray(istream& input) {
            Array result;
            if (input.peek() == -1) throw ParsingError("Array parsing error");

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(std::move(result));
        }




        Node LoadDict(istream& input) {
            Dict result;
            if (input.peek() == -1) throw ParsingError("Array parsing error");

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({ std::move(key), LoadNode(input) });
            }

            return Node(std::move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            //if (!(input >> c)) { // Проверка успешности чтения
                //throw ParsingError("Failed to read character from input");
            //}

            //std::cout << "Read character: " << c << std::endl; // Логирование
            input >> c;
            if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (std::isdigit(c) || c == '-') {
                input.putback(c);
                return LoadNumber(input);
            }
            else {
                throw ParsingError("Unexpected character while parsing JSON");
            }
        }


    }  // namespace


    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return holds_alternative<double>(*this) || holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(*this);
    }

    int Node::AsInt() const {
        if (!IsInt()) throw std::logic_error("wrong type");
        return std::get<int>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) throw std::logic_error("wrong type");
        return std::get<bool>(*this);
    }

    double Node::AsDouble() const
    {
        if (holds_alternative<int>(*this))
        {
            return static_cast<double>(get<int>(*this));
        }
        if (holds_alternative<double>(*this))
        {
            return get<double>(*this);
        }
        throw std::logic_error("Value is not a number");
    }

    const std::string& Node::AsString() const {
        if (!IsString()) throw std::logic_error("wrong type");
        return std::get<std::string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) throw std::logic_error("wrong type");
        return std::get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) throw std::logic_error("wrong type");
        return std::get<Dict>(*this);
    }

    const Value& Node::GetValue() const {
        return *this;
    }

    /*bool Node::operator==(const Node& rhs) const {
        return value_ == rhs.value_;
    }

    bool Node::operator!=(const Node& rhs) const {
        return !(value_ == rhs.value_);
    }*/

    bool Node::operator==(const Node& rhs) const {
        return static_cast<const Value&>(*this) == static_cast<const Value&>(rhs);
    }

    bool Node::operator!=(const Node& rhs) const {
        return !(*this == rhs);
    }

    Document::Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& rhs) const {
        return root_ == rhs.root_;
    }

    bool Document::operator!=(const Document& rhs) const {
        return !(root_ == rhs.root_);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void ValuePrinter::operator()(std::nullptr_t) {
        out << "null"sv;
    }


    void ValuePrinter::operator()(std::string value) {
        out << "\""sv;
        for (const char& c : value) {
            // Экранированные символы
            if (c == '\n') {
                out << "\\n"sv;
            }
            else if (c == '\r') {
                out << "\\r"sv;
            }
            else if (c == '\t') {
                out << "\\t"sv;
            }
            else if (c == '\"') {
                out << "\\\""sv;
            }
            else if (c == '\\') {
                out << "\\\\"sv;
            }
            else {
                out << c;
            }
        }
        out << "\""sv;
    }


    void ValuePrinter::operator()(int value) {
        out << value;
    }

    void ValuePrinter::operator()(double value) {
        out << value;
    }

    void ValuePrinter::operator()(bool value) {
        out << std::boolalpha << value;
    }

    void ValuePrinter::operator()(Array array) {
        out << "[\n"sv;
        indent_level++;
        for (size_t i = 0; i < array.size(); ++i) {
            Indent();
            std::visit(ValuePrinter{ out, indent_level }, array[i].GetValue());
            if (i < array.size() - 1)
            {
                out << ",\n"sv;
            }
        }
        indent_level--;
        out << "\n"sv;
        Indent();
        out << "]"sv;

    }

    void ValuePrinter::operator()(Dict dict) {
        out << "{\n"sv;
        indent_level++;
        for (auto it = dict.begin(); it != dict.end(); ++it) {
            Indent();
            out << "\""sv << it->first << "\": ";
            std::visit(ValuePrinter{ out, indent_level }, it->second.GetValue());
            if (std::next(it) != dict.end()) {
                out << ",\n"sv;
            }
        }
        indent_level--;
        out << "\n"sv;
        Indent();
        out << "}"sv;
    }

    void Print(const Document& doc, std::ostream& out) {
        std::visit(ValuePrinter{ out }, doc.GetRoot().GetValue());
    }

}  // namespace json
