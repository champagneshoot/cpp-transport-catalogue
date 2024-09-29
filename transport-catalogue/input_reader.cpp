#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) 
{
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) 
    {
        return { nan, nan };
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return { lat, lng };
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) 
{
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) 
{
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) 
    {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) 
        {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) 
        {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) 
{
    if (route.find('>') != route.npos) 
    {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) 
{
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) 
    {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) 
    {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) 
    {
        return {};
    }

    return 
    {
        std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)) 
    };
}

void ParseStopWithDistances(std::string stop_name,std::string_view str, TransportCatalogue& transport_catalogue)
{
    
    size_t pos = 0;
    while ((pos = str.find("m to ", pos)) != std::string_view::npos)
    {
        size_t distance_start = str.rfind(' ', pos);
        if (distance_start == std::string_view::npos)
        {
            break;
        }

        int distance = std::stoi(std::string(str.substr(distance_start + 1, pos - distance_start - 1)));
        size_t stop_end = str.find(',', pos);
        if (stop_end == std::string_view::npos)
        {
            stop_end = str.length();
        }

        std::string_view neighbor_stop = Trim(str.substr(pos + 5, stop_end - pos - 5));

        Stop* stop_ptr = transport_catalogue.FindStop(std::string(stop_name));
        Stop* neighbor_stop_ptr = transport_catalogue.FindStop(std::string(neighbor_stop));

        if (stop_ptr && neighbor_stop_ptr) 
        {
            transport_catalogue.AddDistance(*stop_ptr, *neighbor_stop_ptr, distance);
        }

        pos = stop_end + 1;
    }
}


void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const
{
    for (const auto& command : commands_)
    {
        if (command.command == "Stop")
        {
            auto coordinates = ParseCoordinates(command.description);
            catalogue.AddStop(command.id, coordinates);
        }
    }
    for (const auto& command : commands_)
    {
        if (command.command == "Stop")
        {
            ParseStopWithDistances(command.id, command.description, catalogue);
        }
    }
    for (const auto& command : commands_)
    {
        if (command.command == "Bus")
        {
            auto route = ParseRoute(command.description);
            catalogue.AddBus(command.id, route);
        }
    }
}
