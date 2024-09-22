#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const std::string& stop_name, Coordinates coordinates)
{
	stops.push_back({ stop_name, coordinates});
	stopname_to_stop[stops.back().stop_name] = &stops.back();
}

Stop* TransportCatalogue::FindStop(std::string_view stop_name) const
{
    auto it = stopname_to_stop.find(stop_name); 
    if (it != stopname_to_stop.end()) 
    {
        return it->second; 
    }
    return nullptr; 
}

void TransportCatalogue::AddBus(const std::string &bus_name, std::vector<std::string_view> stop_names) 
{
    std::vector<Stop*> stops_on_route;
    for (const auto& stop_name : stop_names) 
    {
        Stop* stop = FindStop(std::string(stop_name));
        if (stop) 
        {
            stops_on_route.push_back(stop);
            stop_to_buses[stop].insert(bus_name);
        }
    }
    buses.push_back({ bus_name, stops_on_route });
    busname_to_bus[buses.back().bus_name] = &buses.back();
}


Bus* TransportCatalogue::FindBus(std::string_view bus_name) const
{
    auto it = busname_to_bus.find(bus_name); 
    if (it != busname_to_bus.end()) 
    {
        return it->second;
    }
    return nullptr;
}

const std::set<std::string>* TransportCatalogue::GetBusesByStop(std::string_view stop_name) const
{
    Stop* stop = FindStop(stop_name);
    if (stop && stop_to_buses.count(stop) > 0) 
    {
        return &stop_to_buses.at(stop);  
    }
    return nullptr; 
}

std::optional<std::tuple<int, int, double>> TransportCatalogue::GetBusInfo(const std::string_view bus_name) const
{
    Bus* bus = FindBus(bus_name);
    if (!bus) 
    {
        return std::nullopt;  
    }

    size_t total_stops = bus->bus_stops.size();
    std::unordered_set<Stop*> unique_stops(bus->bus_stops.begin(), bus->bus_stops.end());
    size_t unique_stop_count = unique_stops.size();
    double route_length = 0.0;
    for (size_t i = 1; i < bus->bus_stops.size(); ++i) 
    {
        route_length += ComputeDistance(bus->bus_stops[i - 1]->stop_coordinates, bus->bus_stops[i]->stop_coordinates);
    }
    return std::make_tuple(total_stops, unique_stop_count, route_length);
}
