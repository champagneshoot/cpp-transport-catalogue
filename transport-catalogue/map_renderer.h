#pragma once

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <unordered_set>


struct Settings
{
    double width;
    double height;

    double padding;

    double stop_radius;
    double line_width;

    int bus_label_font_size;
    svg::Point bus_label_offset;

    int stop_label_font_size;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;

    std::vector<svg::Color> color_palette;
};

inline const double EPSILON = 1e-6;

class SphereProjector
{
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding)
    {
        if (points_begin == points_end) { return; }

        const auto [left_it, right_it] = std::minmax_element(points_begin, points_end,
            [](const geo::Coordinates& lhs, const geo::Coordinates& rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end,
            [](const geo::Coordinates& lhs, const geo::Coordinates& rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_))
        {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat))
        {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom)
        {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom)
        {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom)
        {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const
    {
        return
        {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }
};

class MapRenderer {
public:
    MapRenderer(const Settings& settings) : settings_(settings) {}

    void RenderStopsAndBuses(const TransportCatalogue& catalogue, svg::Document& doc);

private:
    const Settings& settings_;

    struct StopComparator
    {
        bool operator()(const Stop* lhs, const Stop* rhs) const
        {
            return lhs->stop_name < rhs->stop_name;
        }
    };

    void Polyline(const std::vector<const Bus*>& buses, SphereProjector& projector, svg::Document& doc);

    void Circle(std::set<const Stop*, StopComparator>& unique_stops, SphereProjector& projector, svg::Document& doc);

    void RenderBusNames(const std::vector<const Bus*>& buses, SphereProjector& projector, svg::Document& doc);

    void DrawBusName(const svg::Point& position, const std::string& bus_name, const svg::Color& bus_color, svg::Document& doc);

    void RenderStopNames(std::set<const Stop*, StopComparator>& unique_stops, SphereProjector& projector, svg::Document& doc);
};
