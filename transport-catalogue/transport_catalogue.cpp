#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const std::string& stop_name, Coordinates coordinates)
{
    stops.push_back({ stop_name, coordinates });
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

void TransportCatalogue::AddDistance(const Stop* from, const Stop* to, int distance) 
{
    distances_[std::make_pair(from, to)] = distance;
}

int TransportCatalogue::CalculateFullRouteLength(const Bus* bus) const
{
    int full_route_length = 0;
    for (size_t i = 1; i < bus->bus_stops.size(); ++i)
    {
        auto it = distances_.find({ bus->bus_stops[i - 1], bus->bus_stops[i] });
        if (it != distances_.end())
        {
            full_route_length += it->second;
        }
        else
        {
            auto it_reversed = distances_.find({ bus->bus_stops[i], bus->bus_stops[i - 1] });
            if (it_reversed != distances_.end())
            {
                full_route_length += it_reversed->second;
            }
        }
    }
    return full_route_length;
}


std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string_view bus_name) const
{
    BusInfo bus_info;
    Bus* bus = FindBus(bus_name);
    if (!bus) 
    {
        return std::nullopt;  
    }
    bus_info.total_stops = bus->bus_stops.size();
    std::unordered_set<Stop*> unique_stops(bus->bus_stops.begin(), bus->bus_stops.end());
    bus_info.unique_stops = unique_stops.size();
    bus_info.full_route_length = CalculateFullRouteLength(bus);
    double route_length = 0.0;
    for (size_t i = 1; i < bus->bus_stops.size(); ++i) 
    {
        route_length += ComputeDistance(bus->bus_stops[i - 1]->stop_coordinates, bus->bus_stops[i]->stop_coordinates);
    }
    bus_info.curvature = static_cast<double>(bus_info.full_route_length) / route_length;
    return bus_info;
}

size_t Hasher::operator()(const std::pair<const Stop*, const Stop*>& stop_pair) const
{
    std::hash <const Stop*> ptr_hasher;
    size_t hash1 = ptr_hasher(stop_pair.first);
    size_t hash2 = ptr_hasher(stop_pair.second);
    return hash1 + hash2 * 8 ;
}
