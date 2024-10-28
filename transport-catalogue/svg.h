#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <string_view>
#include <optional>
#include <variant>


namespace svg
{
    struct Rgb
    {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    struct Rgba
    {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    class Color
    {
    public:
        // Конструкторы для разных типов
        Color() : color_(std::monostate{}) {}
        Color(const std::string& color) : color_(color) {}
        Color(const Rgb& rgb) : color_(rgb) {}
        Color(const Rgba& rgba) : color_(rgba) {}

        // Вывод цвета в ostream
        friend std::ostream& operator<<(std::ostream& os, const Color& color) {
            std::visit([&os](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    os << "none";
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    os << arg;
                }
                else if constexpr (std::is_same_v<T, Rgb>) {
                    os << "rgb(" << static_cast<int>(arg.red) << ","
                        << static_cast<int>(arg.green) << ","
                        << static_cast<int>(arg.blue) << ")";
                }
                else if constexpr (std::is_same_v<T, Rgba>) {
                    os << "rgba(" << static_cast<int>(arg.red) << ","
                        << static_cast<int>(arg.green) << ","
                        << static_cast<int>(arg.blue) << ","
                        << arg.opacity << ")";
                }
                }, color.color_);
            return os;
        }

    private:
        std::variant<std::monostate, std::string, Rgb, Rgba> color_;
    };
    const Color NoneColor = Color();


    enum class StrokeLineCap
    {
        BUTT,
        ROUND,
        SQUARE,
    };
    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap);

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join);

    struct Point
    {
        Point() = default;
        Point(double x, double y) : x(x), y(y) {}
        double x = 0;
        double y = 0;
    };
    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext
    {
        RenderContext(std::ostream& out) : out(out) {}

        RenderContext(std::ostream& out, int indent_step, int indent = 0) :out(out), indent_step(indent_step), indent(indent) {}

        RenderContext Indented() const
        {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    template <typename Owner>
    class PathProps
    {
    public:
        template <typename T>
        Owner& SetFillColor(T&& color)
        {
            fill_color_ = Color(std::forward<T>(color));
            return AsOwner();
        }
        template <typename T>
        Owner& SetStrokeColor(T&& color)
        {
            stroke_color_ = Color(std::forward<T>(color));
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width)
        {
            width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap)
        {
            line_cap_ = line_cap;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join)
        {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:

        // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
        void RenderAttrs(std::ostream& out) const
        {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_)
            {
                out << " stroke-width=\"" << *width_ << "\"";
            }
            if (line_cap_)
            {
                out << " stroke-linecap=\"" << *line_cap_ << "\"";
            }
            if (line_join_)
            {
                out << " stroke-linejoin=\"" << *line_join_ << "\"";
            }
        }

    private:
        Owner& AsOwner()
        {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object
    {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle : public Object, public PathProps<Circle>
    {
    public:
        Circle() :center_({ 0.0,0.0 }), radius_(1.0) {}
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);
    private:
        void RenderObject(const RenderContext& context) const;
        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline>
    {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline() : points_() {}

        Polyline& AddPoint(Point point)
        {
            points_.push_back(point);
            return *this;
        }
        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const
        {
            context.RenderIndent();
            context.out << "<polyline points=\"";
            for (size_t i = 0; i < points_.size(); ++i) {
                const auto& point = points_[i];
                context.out << point.x << "," << point.y;
                // Проверяем, не является ли текущая точка последней
                if (i != points_.size() - 1) {
                    context.out << " "; // Добавляем запятую и пробел только если это не последняя точка
                }
            }
            //RenderAttrs(context.out);
            context.out << "\"";
            RenderAttrs(context.out);
            context.out << "/>";

        }
        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text : public Object, public PathProps<Text>
    {
    public:
        Text() : pos_({ 0.0, 0.0 }), offset_({ 0.0, 0.0 }), size_(1), data_() {}
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos)
        {
            pos_ = pos;
            return *this;
        }

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset)
        {
            offset_ = offset;
            return *this;
        }

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size)
        {
            size_ = size;
            return *this;
        }

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family)
        {
            font_family_ = font_family;
            return *this;
        }

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight)
        {
            font_weight_ = font_weight;
            return *this;
        }

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data)
        {
            data_ = data;
            return *this;
        }

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        Point pos_;
        Point offset_;
        uint32_t size_;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
        void RenderObject(const RenderContext& context) const
        {
            context.RenderIndent();

            context.out << "<text";
            RenderAttrs(context.out);
            context.out << " x=\"" << pos_.x << "\" y=\"" << pos_.y
                << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y
                << "\" font-size=\"" << size_ << "\"";

            // Выводим font-family и font-weight только если они заданы
            if (!font_family_.empty())
                context.out << " font-family=\"" << font_family_ << "\"";
            if (!font_weight_.empty())
                context.out << " font-weight=\"" << font_weight_ << "\"";

            context.out << ">";
            for (const char c : data_)
            {
                if (c == '"')
                {
                    context.out << "&quot;";
                }
                else if (c == '\'')
                {
                    context.out << "&apos;";
                }
                else if (c == '<')
                {
                    context.out << "&lt;";
                }
                else if (c == '>')
                {
                    context.out << "&gt;";
                }
                else if (c == '&')
                {
                    context.out << "&amp;";
                }
                else
                {
                    context.out << c;
                }
            }
            context.out << "</text>";
        }
    };

    class ObjectContainer
    {
    public:
        template <typename Obj>
        void Add(Obj object)
        {
            objects_.emplace_back(std::make_unique<Obj>(std::move(object)));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    protected:
        ~ObjectContainer() = default;
        std::vector<std::unique_ptr<Object>> objects_;

    };

    class Drawable
    {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer
    {
    public:

        //Document() :{};
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj)
        {
            objects_.emplace_back(std::move(obj));
        }

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const
        {
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
            RenderContext context(out);
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
            for (const auto& obj : objects_) {
                obj->Render(context);
            }
            out << "</svg>\n";
        }

        // Прочие методы и данные, необходимые для реализации класса Document

    };

}  // namespace svg
