#include "stat_reader.h"

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) 
{
    if (request.substr(0, 4) == "Bus ") 
    {
        std::string bus_name = std::string(request.substr(4));
        auto bus_info_opt = transport_catalogue.GetBusInfo(bus_name);
        if (bus_info_opt)
        {
            const auto& bus_info = *bus_info_opt;
            output << "Bus " << bus_name << ": " << bus_info.total_stops << " stops on route, "
                << bus_info.unique_stops << " unique stops, " << bus_info.full_route_length << " route length, " << bus_info.curvature << " curvature\n";
        }
        else 
        {
            output << "Bus " << bus_name << ": not found\n";
        }
    }
    else if (request.substr(0, 5) == "Stop ") {
        std::string stop_name = std::string(request.substr(5));
        Stop* stop = transport_catalogue.FindStop(stop_name);
        if (stop) 
        {
            const auto* buses = transport_catalogue.GetBusesByStop(stop_name);
            if (buses && !buses->empty()) 
            {
                output << "Stop " << stop_name << ": buses";
                for (const auto& bus : *buses) 
                {
                    output << " " << bus;  
                }
                output << "\n";
            }
            else 
            {
                output << "Stop " << stop_name << ": no buses\n";
            }
        }
        else 
        {
            output << "Stop " << stop_name << ": not found\n";
        }
    }
    else 
    {
        // Если запрос не распознан
        output << "Unknown request type\n";
    }
}
