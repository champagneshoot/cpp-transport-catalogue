#pragma once
#include "geo.h"

#include <algorithm>
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <set>
#include <unordered_set>
#include <iostream>
#include <vector>


struct Stop
{
	std::string stop_name;
	geo::Coordinates stop_coordinates;
};
struct Bus
{
	std::string bus_name;
	std::vector<Stop*> bus_stops;
	bool is_roundtrip;
};
class Hasher
{
public:
	size_t operator()(const std::pair<const Stop*, const Stop*>& stop_pair) const;
};
struct BusInfo
{
	int total_stops;
	int unique_stops;
	int full_route_length;
	double curvature;
};
class TransportCatalogue
{
public:
	void AddStop(const std::string&, geo::Coordinates);
	Stop* FindStop(std::string_view) const;
	void AddBus(const std::string&, std::vector<std::string_view>, bool);
	Bus* FindBus(std::string_view) const;
	std::optional<BusInfo> GetBusInfo(const std::string_view) const;
	const std::set<std::string>* GetBusesByStop(std::string_view) const;

	void AddDistance(const Stop*, const Stop*, int);
	int CalculateFullRouteLength(const Bus* bus) const;
	int RouteLenghtBetweenTwoStops(const Stop*, const Stop*) const;


	std::vector<const Bus*> GetAllBuses() const
	{
		std::vector<const Bus*> all_buses;
		all_buses.reserve(buses.size());
		for (const auto& bus : buses)
		{
			all_buses.push_back(&bus);
		}
		std::sort(all_buses.begin(), all_buses.end(), [](const Bus* a, const Bus* b) { return a->bus_name < b->bus_name; });
		return all_buses;
	}

	std::vector<const Stop*> GetAllStops() const
	{
		std::vector<const Stop*> all_stops;
		all_stops.reserve(stops.size());
		for (const auto& stop : stops)
		{
			all_stops.push_back(&stop);
		}
		return all_stops;
	}

private:
	std::deque<Stop> stops;
	std::unordered_map<std::string_view, Stop*> stopname_to_stop;
	std::deque <Bus> buses;
	std::unordered_map<std::string_view, Bus*> busname_to_bus;
	std::unordered_map<Stop*, std::set<std::string>> stop_to_buses;
	std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher> distances_;
};
