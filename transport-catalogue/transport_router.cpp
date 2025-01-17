#include "transport_router.h"

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, int bus_wait_time, double bus_velocity)
    : catalogue_(catalogue), bus_wait_time_(bus_wait_time), bus_velocity_(bus_velocity)
{
    InitializeStops();
    AddBusEdges();
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

void TransportRouter::InitializeStops() {
    const auto& stops = catalogue_.GetStopNameToStopMap();
    size_t vertex_count = stops.size() * 2;
    graph_ = graph::DirectedWeightedGraph<double>(vertex_count);

    graph::VertexId vertex_id = 0;
    for (const auto& [stop_name, stop_info] : stops) {
        stop_ids_[stop_name] = vertex_id;

        graph_.AddEdge(graph::Edge<double>{stop_info->stop_name, 0, vertex_id, vertex_id + 1, static_cast<double>(bus_wait_time_) });

        vertex_id += 2;
    }
}

void TransportRouter::AddBusEdges() {
    const auto& buses = catalogue_.GetBusNameToBusMap();

    for (const auto& [bus_name, bus_info] : buses) 
    {
        const auto& stops = bus_info->bus_stops;
        size_t stop_count = stops.size();

        for (size_t i = 0; i + 1 < stop_count; ++i) 
        {
            for (size_t j = i + 1; j < stop_count; ++j)
            {
                size_t span_count = j - i;

                double total_distance_forward = 0.0;
                for (size_t k = i + 1; k <= j; ++k) {
                    total_distance_forward += catalogue_.RouteLenghtBetweenTwoStops(stops[k - 1], stops[k]);
                }

                double travel_time_forward = total_distance_forward / (bus_velocity_ * (1000.0 / 60.0));

                graph_.AddEdge(graph::Edge<double>{static_cast<std::string>(bus_name), span_count, stop_ids_.at(stops[i]->stop_name) + 1, 
                    stop_ids_.at(stops[j]->stop_name), travel_time_forward});

                if (!bus_info->is_roundtrip) {
                    double total_distance_backward = 0.0;
                    for (size_t k = j; k > i; --k) {
                        total_distance_backward += catalogue_.RouteLenghtBetweenTwoStops(stops[k], stops[k - 1]);
                    }
                    double travel_time_backward = total_distance_backward / (bus_velocity_ * (1000.0 / 60.0));
                    graph_.AddEdge(graph::Edge<double>{static_cast<std::string>(bus_name), span_count, 
                        stop_ids_.at(stops[j]->stop_name) + 1, stop_ids_.at(stops[i]->stop_name), travel_time_backward});
                }
            }
        }
    }
}

std::optional<RouteResult> TransportRouter::FindRoute(std::string_view stop_from, std::string_view stop_to) const {
    auto from_it = stop_ids_.find(stop_from);
    auto to_it = stop_ids_.find(stop_to);

    if (from_it == stop_ids_.end() || to_it == stop_ids_.end()) {
        return std::nullopt;
    }

    auto route_info = router_->BuildRoute(from_it->second, to_it->second);

    if (!route_info) {
        return std::nullopt;
    }

    RouteResult result;
    result.total_time = route_info->weight;

    for (const auto& edge_id : route_info->edges) {
        const auto& edge = graph_.GetEdge(edge_id);

        RouteItem item;

        if (edge.quality == 0) 
        { 
            item.type = RouteItem::ItemType::Wait;
            item.name = edge.name; 
            item.time = edge.weight;
        }
        else 
        {
            item.type = RouteItem::ItemType::Bus;
            item.name = edge.name; 
            item.span_count = edge.quality;
            item.time = edge.weight;
        }
        result.items.push_back(std::move(item));
    }

    return result;
}





