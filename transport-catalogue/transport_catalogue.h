#pragma once
#include "geo.h"
#include <iostream>
#include "input_reader.h"
#include <deque>
#include<unordered_map>
#include <string>
#include <string_view>
#include<optional>
#include <set>
#include <unordered_set>

struct Stop
{
	std::string stop_name;
	Coordinates stop_coordinates;
};
struct Bus
{
	std::string bus_name;
	std::vector<Stop*> bus_stops;
};
class TransportCatalogue 
{
public:
	void AddStop(const std::string, Coordinates);
	Stop* FindStop(std::string) const;
	void AddBus(const std::string&, std::vector<std::string_view>);
	Bus* FindBus(std::string) const;
	std::optional<std::tuple<int, int, double>> GetBusInfo(const std::string) const;
	const std::set<std::string>* GetBusesByStop(std::string) const;
private:
	std::deque<Stop> stops;
	std::unordered_map<std::string_view, Stop*> stopname_to_stop;
	std::deque <Bus> buses;
	std::unordered_map<std::string_view, Bus*> busname_to_bus;
	std::unordered_map<Stop*, std::set<std::string>> stop_to_buses;
};
