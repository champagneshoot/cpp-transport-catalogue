#pragma once
#include <vector>
#include <string>
#include <sstream>

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

class InformationProcessing
{
public:

    InformationProcessing(TransportCatalogue& catalogue, std::istream& input_stream_, std::ostream& out_);

    void Process()
    {
        const auto& base_requests = root.AsMap().at("base_requests").AsArray();
        ProcessBaseRequests(base_requests);

        const auto& render_settings = root.AsMap().at("render_settings").AsMap();
        ProcessRendererSet(render_settings);

        const auto& routing_settings = root.AsMap().at("routing_settings").AsMap();
        ProcessRoutingSettings(routing_settings);

        svg::Document svg_doc;
        MapRenderer mr(set);
        mr.RenderStopsAndBuses(catalogue_, svg_doc);
        svg_doc.Render(os);
    }

    void ProcessRequest()
    {
        const auto& stat_requests = root.AsMap().at("stat_requests").AsArray();
        if (!stat_requests.empty())
            ProcessStatRequests(stat_requests);
    }

    void ProcessBaseRequests(const json::Array& base_requests);
    void ProcessStatRequests(const json::Array& stat_requests);
    void ProcessRendererSet(const json::Dict& renderer_settings);
    void ProcessRoutingSettings(const json::Dict& routing_settings);

private:
    TransportCatalogue catalogue_;
    Settings set;
    std::optional<TransportRouter> transport_router_;

    int bus_wait_time_ = 0;
    double bus_velocity_ = 0.0;

    std::istream& input_stream;
    std::ostream& out;
    std::ostringstream os;

    json::Document doc;
    json::Node root;

    svg::Color ProcessColor(const json::Node& color_node);

    void ProcessStop(const json::Dict& stop_data);
    void ProcessStopWithDistance(const json::Dict& stop_data);
    void ProcessBus(const json::Dict& bus_data);

    void ProcessStopRequest(const json::Dict& stop_request, json::Array& response_array);
    void ProcessBusRequest(const json::Dict& bus_request, json::Array& response_array);
    void ProcessMapRequest(const json::Dict& map_request, json::Array& response_array);
    void ProcessRouteRequest(const json::Dict& route_request, json::Array& response_array);
};


