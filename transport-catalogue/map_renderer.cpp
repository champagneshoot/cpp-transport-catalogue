#include "map_renderer.h"

void MapRenderer::RenderStopsAndBuses(const TransportCatalogue& catalogue, svg::Document& doc)
{
    auto buses = catalogue.GetAllBuses();
    std::vector<geo::Coordinates> stop_coordinates;
    std::set<const Stop*, StopComparator> unique_stops;

    // Собираем координаты только тех остановок, которые входят в маршруты
    for (const auto& bus : buses)
    {
        for (const auto& stop_ptr : bus->bus_stops)
        {

            if (unique_stops.insert(stop_ptr).second)
            {
                stop_coordinates.push_back(stop_ptr->stop_coordinates);
            }
        }
    }

    std::sort(buses.begin(), buses.end(), [](const auto& lhs, const auto& rhs) {
        return lhs->bus_name < rhs->bus_name;
        });

    SphereProjector projector(stop_coordinates.begin(), stop_coordinates.end(), settings_.width, settings_.height, settings_.padding);

    // Отрисовка маршрутов
    Polyline(buses, projector, doc);

    // Отрисовка названий маршрутов
    RenderBusNames(buses, projector, doc);

    // Отрисовка точек
    Circle(unique_stops, projector, doc);

    // Отрисовка названий остановок
    RenderStopNames(unique_stops, projector, doc);
}

void MapRenderer::Polyline(const std::vector<const Bus*>& buses, SphereProjector& projector, svg::Document& doc)
{
    size_t color_index = 0;

    for (const auto& bus : buses) {
        if (bus->bus_stops.empty()) {
            continue;
        }

        svg::Polyline polyline;
        for (const auto& stop_ptr : bus->bus_stops) {
            polyline.AddPoint(projector(stop_ptr->stop_coordinates));
        }

        polyline.SetFillColor(svg::NoneColor)
            .SetStrokeColor(settings_.color_palette[color_index])
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND);

        doc.AddPtr(std::make_unique<svg::Polyline>(polyline));
        color_index = (color_index + 1) % settings_.color_palette.size();
    }
}

void MapRenderer::RenderBusNames(const std::vector<const Bus*>& buses, SphereProjector& projector, svg::Document& doc)
{
    size_t color_index = 0;

    for (const auto& bus : buses)
    {
        if (bus->bus_stops.empty())
        {
            continue;
        }

        const Stop* first_stop = bus->bus_stops.front();
        const Stop* last_stop = bus->bus_stops.front();

        if (bus->is_roundtrip == false)
            last_stop = bus->bus_stops[bus->bus_stops.size() / 2];

        auto first_stop_position = projector(first_stop->stop_coordinates);
        DrawBusName(first_stop_position, bus->bus_name, settings_.color_palette[color_index], doc);


        if (last_stop != first_stop)
        {
            auto last_stop_position = projector(last_stop->stop_coordinates);
            DrawBusName(last_stop_position, bus->bus_name, settings_.color_palette[color_index], doc);
        }
        color_index = (color_index + 1) % settings_.color_palette.size();
    }
}

void MapRenderer::DrawBusName(const svg::Point& position, const std::string& bus_name, const svg::Color& bus_color, svg::Document& doc)
{
    // Создаем подложку
    svg::Text underlayer;
    underlayer.SetPosition(position)
        .SetOffset(settings_.bus_label_offset)
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
        .SetData(bus_name);

    doc.AddPtr(std::make_unique<svg::Text>(underlayer));

    // Создаем надпись
    svg::Text text;
    text.SetPosition(position)
        .SetOffset(settings_.bus_label_offset)
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetFillColor(bus_color)
        .SetData(bus_name);

    doc.AddPtr(std::make_unique<svg::Text>(text));
}

void MapRenderer::Circle(std::set<const Stop*, StopComparator>& unique_stops, SphereProjector& projector, svg::Document& doc)
{
    for (const auto& stop_ptr : unique_stops)
    {
        svg::Circle circle;
        circle.SetCenter(projector(stop_ptr->stop_coordinates))
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white");
        doc.AddPtr(std::make_unique<svg::Circle>(circle));
    }
}

void MapRenderer::RenderStopNames(std::set<const Stop*, StopComparator>& unique_stops, SphereProjector& projector, svg::Document& doc)
{
    for (const auto& stop_ptr : unique_stops)
    {
        svg::Text text;

        auto stop_name_position = projector(stop_ptr->stop_coordinates);


        // Создаем подложку
        svg::Text underlayer;
        underlayer.SetPosition(stop_name_position)
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetData(stop_ptr->stop_name);

        doc.AddPtr(std::make_unique<svg::Text>(underlayer));

        // Создаем надпись
        text.SetPosition(stop_name_position)
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFillColor("black")
            .SetFontFamily("Verdana")
            .SetData(stop_ptr->stop_name);

        doc.AddPtr(std::make_unique<svg::Text>(text));
    }
}
