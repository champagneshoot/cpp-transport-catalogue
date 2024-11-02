#include "json_reader.h"

InformationProcessing::InformationProcessing(TransportCatalogue& catalogue, std::istream& input_stream_, std::ostream& out_)
    : catalogue_(catalogue), input_stream(input_stream_), out(out_)
{
    doc = json::Load(input_stream);
    root = doc.GetRoot();
}



void InformationProcessing::ProcessBaseRequests(const json::Array& base_requests)
{
    for (const auto& request : base_requests)
    {
        const auto& type = request.AsMap().at("type").AsString();
        if (type == "Stop")
        {
            ProcessStop(request.AsMap());
        }
    }

    for (const auto& request : base_requests)
    {
        const auto& type = request.AsMap().at("type").AsString();
        if (type == "Stop")
        {
            ProcessStopWithDistance(request.AsMap());
        }
    }

    for (const auto& request : base_requests)
    {
        const auto& type = request.AsMap().at("type").AsString();
        if (type == "Bus")
        {
            ProcessBus(request.AsMap());
        }
    }
}

svg::Color InformationProcessing::ProcessColor(const json::Node& color_node)
{
    if (color_node.IsString())
    {
        return svg::Color(color_node.AsString());
    }
    else if (color_node.IsArray()) 
    {
        const auto& color_array = color_node.AsArray();
        if (color_array.size() == 3) 
        {

            return svg::Rgb
            {
                static_cast<uint8_t>(color_array[0].AsInt()),
                    static_cast<uint8_t>(color_array[1].AsInt()),
                    static_cast<uint8_t>(color_array[2].AsInt())
            };
        }
        else if (color_array.size() == 4)
        {
            return svg::Rgba
            {
                static_cast<uint8_t>(color_array[0].AsInt()),
                    static_cast<uint8_t>(color_array[1].AsInt()),
                    static_cast<uint8_t>(color_array[2].AsInt()),
                    color_array[3].AsDouble()
            };
        }
    }
    throw std::logic_error("Invalid color format");
}

void InformationProcessing::ProcessStop(const json::Dict& stop_data)
{
    const std::string& name = stop_data.at("name").AsString();
    double latitude = stop_data.at("latitude").AsDouble();
    double longitude = stop_data.at("longitude").AsDouble();
    catalogue_.AddStop(name, { latitude, longitude });
}

void InformationProcessing::ProcessStopWithDistance(const json::Dict& stop_data)
{
    const std::string& name = stop_data.at("name").AsString();
    const auto& distances = stop_data.at("road_distances").AsMap();

    Stop* new_stop = catalogue_.FindStop(name);
    for (const auto& [stop_name, distance_node] : distances)
    {
        int distance = distance_node.AsInt();
        Stop* other_stop = catalogue_.FindStop(stop_name);
        if (other_stop)
        {
            catalogue_.AddDistance(new_stop, other_stop, distance);
        }
    }
}

void InformationProcessing::ProcessBus(const json::Dict& bus_data)
{
    const std::string& name = bus_data.at("name").AsString();
    const auto& stops_array = bus_data.at("stops").AsArray();
    bool is_roundtrip = bus_data.at("is_roundtrip").AsBool();

    std::vector<std::string_view> stops;
    for (const auto& stop_node : stops_array)
    {
        stops.push_back(stop_node.AsString());
    }
    if (!is_roundtrip)
    {
        // Для некольцевого маршрута (A-B-C-D) - [A,B,C,D,C,B,A]
        std::vector<std::string_view> full_stops;
        full_stops.insert(full_stops.end(), stops.begin(), stops.end());
        for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it)
        {
            full_stops.push_back(*it);
        }
        stops = std::move(full_stops);
    }
    catalogue_.AddBus(name, stops, is_roundtrip);
}

void InformationProcessing::ProcessStatRequests(const json::Array& stat_requests)
{
    json::Array response_array;

    for (const auto& request : stat_requests)
    {
        const auto& type = request.AsMap().at("type").AsString();
        if (type == "Stop")
        {
            ProcessStopRequest(request.AsMap(), response_array);
        }
        else if (type == "Bus")
        {
            ProcessBusRequest(request.AsMap(), response_array);
        }
        else if (type == "Map")
        {
            ProcessMapRequest(request.AsMap(), response_array);
        }
    }
    json::Document doc(json::Node(std::move(response_array)));
    json::Print(doc, out);
}

void InformationProcessing::ProcessRendererSet(const json::Dict& renderer_settings)
{
    set.width = renderer_settings.at("width").AsDouble();
    set.height = renderer_settings.at("height").AsDouble();

    set.padding = renderer_settings.at("padding").AsDouble();

    set.line_width = renderer_settings.at("line_width").AsDouble();
    set.stop_radius = renderer_settings.at("stop_radius").AsDouble();

    set.bus_label_font_size = renderer_settings.at("bus_label_font_size").AsDouble();
    set.bus_label_offset = { renderer_settings.at("bus_label_offset").AsArray()[0].AsDouble(), renderer_settings.at("bus_label_offset").AsArray()[1].AsDouble() };

    set.stop_label_font_size = renderer_settings.at("stop_label_font_size").AsDouble();
    set.stop_label_offset = { renderer_settings.at("stop_label_offset").AsArray()[0].AsDouble(), renderer_settings.at("stop_label_offset").AsArray()[1].AsDouble() };


    const auto& underlayer_color_json = renderer_settings.at("underlayer_color");
    set.underlayer_color = ProcessColor(underlayer_color_json);

    set.underlayer_width = renderer_settings.at("underlayer_width").AsDouble();

    const auto& color_palette_json = renderer_settings.at("color_palette").AsArray();
    for (const auto& color : color_palette_json)
    {
        set.color_palette.push_back(ProcessColor(color));
    }
}

void InformationProcessing::ProcessStopRequest(const json::Dict& stop_request, json::Array& response_array)
{
    const std::string name = stop_request.at("name").AsString();
    int id = stop_request.at("id").AsInt();
    const std::set<std::string>* buses_ptr = catalogue_.GetBusesByStop(name);
    json::Dict response;
    response["request_id"] = id;

    if (catalogue_.FindStop(name) == NULL)
    {
        response["error_message"] = std::string("not found");
    }

    else if (buses_ptr && !buses_ptr->empty())
    {
        json::Array buses_array;
        for (const auto& bus : *buses_ptr)
        {
            buses_array.push_back(bus);
        }
        response["buses"] = std::move(buses_array);

    }
    else
    {
        json::Array buses_array = {};
        response["buses"] = buses_array;
    }
    response_array.emplace_back(std::move(response));
}

void InformationProcessing::ProcessBusRequest(const json::Dict& bus_request, json::Array& response_array)
{
    const std::string name = bus_request.at("name").AsString();
    int id = bus_request.at("id").AsInt();
    auto bus_info_opt = catalogue_.GetBusInfo(name);

    json::Dict response;
    response["request_id"] = id;
    if (bus_info_opt)
    {
        const auto& bus_info = *bus_info_opt;
        response["curvature"] = bus_info.curvature;
        response["route_length"] = bus_info.full_route_length;
        response["stop_count"] = bus_info.total_stops;
        response["unique_stop_count"] = bus_info.unique_stops;
    }
    else
    {
        response["error_message"] = std::string("not found");
    }
    response_array.emplace_back(std::move(response));
}

void InformationProcessing::ProcessMapRequest(const json::Dict& map_request, json::Array& response_array)
{
    int id = map_request.at("id").AsInt();

    json::Dict response;
    response["request_id"] = id;
    response["map"] = os.str();
    response_array.emplace_back(std::move(response));
}




