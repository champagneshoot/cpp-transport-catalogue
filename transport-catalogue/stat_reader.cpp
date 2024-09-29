#include "stat_reader.h"

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) 
{
    if (request.substr(0, 4) == "Bus ") 
    {
        std::string bus_name = std::string(request.substr(4));
        if (auto bus_info = transport_catalogue.GetBusInfo(bus_name)) 
        {
            int total_stops = std::get<0>(*bus_info);
            int unique_stops = std::get<1>(*bus_info);
            int full_route_length = std::get<2>(*bus_info);
            double route_length = std::get<3>(*bus_info);
            output << "Bus " << bus_name << ": " << total_stops << " stops on route, "
                << unique_stops << " unique stops, " << full_route_length << " route length, " << static_cast<double>(full_route_length)/route_length<<" curvature\n";
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
