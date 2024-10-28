#include "svg.h"

namespace svg
{

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const
    {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    std::ostream& operator<<(std::ostream& out, svg::StrokeLineCap stroke_line_cap) {
        switch (stroke_line_cap) {
        case svg::StrokeLineCap::BUTT:
            return out << "butt";
        case svg::StrokeLineCap::ROUND:
            return out << "round";
        case svg::StrokeLineCap::SQUARE:
            return out << "square";
        default:
            throw std::invalid_argument("Unknown StrokeLineCap value");
        }
    }

    std::ostream& operator<<(std::ostream& out, svg::StrokeLineJoin stroke_line_join) {
        switch (stroke_line_join) {
        case svg::StrokeLineJoin::ARCS:
            return out << "arcs";
        case svg::StrokeLineJoin::BEVEL:
            return out << "bevel";
        case svg::StrokeLineJoin::MITER:
            return out << "miter";
        case svg::StrokeLineJoin::MITER_CLIP:
            return out << "miter-clip";
        case svg::StrokeLineJoin::ROUND:
            return out << "round";
        default:
            throw std::invalid_argument("Unknown StrokeLineJoin value");
        }
    }


}  // namespace svg
