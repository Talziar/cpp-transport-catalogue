#include "json_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std;

namespace transport_catalogue::json_reader {
    using namespace json;

    // ---------- Parsing Helpers----------

    string JsonReader::ParseType(const Dict &dict_node) {
        return dict_node.at("type"s).AsString();
    }

    string JsonReader::ParseName(const Dict &dict_node) {
        return dict_node.at("name"s).AsString();
    }

    geo::Coordinates JsonReader::ParseCoordinates(const Dict &dict_node) {
        return {dict_node.at("latitude"s).AsDouble(), dict_node.at("longitude"s).AsDouble()};
    }

    bool JsonReader::ParseRouteIsRoundtrip(const Dict &dict_node) {
        return dict_node.at("is_roundtrip"s).AsBool();
    }

    vector<string> JsonReader::ParseRoute(const Dict &dict_node) {
        vector<string> results;
        for (auto el : dict_node.at("stops"s).AsArray()) {
            results.push_back(el.AsString());
        }

        if (!ParseRouteIsRoundtrip(dict_node)) {
            for (size_t i = results.size() - 1; i > 0; --i) {
                results.push_back(results[i - 1]);
            }
        }

        return results;
    }

    vector<pair<string, uint32_t>> JsonReader::ParseStopDistances(const Dict &dict_node) {
        auto road_to_distances = dict_node.at("road_distances"s).AsMap();
        vector<pair<string, uint32_t>> road_distances;

        for (auto [stop_name, distance] : road_to_distances) {
            road_distances.emplace_back(stop_name, distance.AsInt());
        }
        return road_distances;
    }

    svg::Color JsonReader::ParseColor(const Node &json_string_or_array) {
        if (json_string_or_array.IsString()) {
            return {json_string_or_array.AsString()};
        } else {
            auto json_array = json_string_or_array.AsArray();
            if (json_array.size() == 3) {
                return svg::Rgb{static_cast<uint16_t>(json_array[0].AsInt()), static_cast<uint16_t>(json_array[1].AsInt()), static_cast<uint16_t>(json_array[2].AsInt())};
            } else {
                return svg::Rgba{static_cast<uint16_t>(json_array[0].AsInt()), static_cast<uint16_t>(json_array[1].AsInt()), static_cast<uint16_t>(json_array[2].AsInt()), json_array[3].AsDouble()};
            }
        }
    }

    // ---------- Print Helpers----------

    Node JsonReader::StopRequestFormat(const RequestHandler &r_h, Dict stat_request) {
        const auto stop_name = stat_request.at("name"s).AsString();
        const auto stop_buses = r_h.GetBusesByStop(stop_name);

        if (!stop_buses.has_value()) {
            return Builder{}.StartDict().Key("request_id"s).Value(stat_request.at("id"s).AsInt()).Key("error_message"s).Value("not found"s).EndDict().Build();
        } else {
            Array stop_buses_vector;
            for (const auto el : *stop_buses.value()) {
                stop_buses_vector.emplace_back(el->first.bus_name);
            }

            return Builder{}.StartDict().Key("request_id"s).Value(stat_request.at("id"s).AsInt()).Key("buses"s).Value(stop_buses_vector).EndDict().Build();
        }
    }

    Node JsonReader::BusRequestFormat(const RequestHandler &r_h, Dict stat_request) {
        const auto bus_name = stat_request.at("name"s).AsString();
        const auto req_bus_info = r_h.GetBusInfo(bus_name);

        if (!req_bus_info) {
            return Builder{}.StartDict().Key("request_id"s).Value(stat_request.at("id"s).AsInt()).Key("error_message"s).Value("not found"s).EndDict().Build();
        } else {
            return Builder{}.StartDict().Key("request_id"s).Value(stat_request.at("id"s).AsInt()).Key("curvature"s).Value(req_bus_info->curvature_).Key("route_length"s).Value(int(req_bus_info->route_length_)).Key("stop_count"s).Value(int(req_bus_info->stop_count_)).Key("unique_stop_count"s).Value(int(req_bus_info->unique_stop_count_)).EndDict().Build();
        }
    }

    Node JsonReader::MapRequestFormat(const RequestHandler &r_h, Dict stat_request) {
        ostringstream s_stream;
        r_h.RenderMap(s_stream);
        return Builder{}.StartDict().Key("request_id"s).Value(stat_request.at("id"s).AsInt()).Key("map"s).Value(s_stream.str()).EndDict().Build();
    }

    // ---------- JsonReader ----------

    Array JsonReader::GetBaseRequests() const {
        return jsonDocument_.GetRoot().AsMap().at("base_requests").AsArray();
    }

    Array JsonReader::GetStatRequests() const {
        return jsonDocument_.GetRoot().AsMap().at("stat_requests").AsArray();
    }

    Dict JsonReader::GetRenderSettings() const {
        return jsonDocument_.GetRoot().AsMap().at("render_settings").AsMap();
    }

    void JsonReader::ApplyBaseRequests(TransportCatalogue &catalogue) const {
        vector<string> request_priority{"Stop"s, "Bus"s};
        const auto base_requests = GetBaseRequests();

        for (const auto &request_type : request_priority) {
            for (const auto &request : base_requests) {
                const auto cur_type = ParseType(request.AsMap());

                if (cur_type != request_type) {
                    continue;
                } else if (cur_type == "Stop"s) {
                    catalogue.AddStop(ParseName(request.AsMap()),
                                      ParseCoordinates(request.AsMap()));
                } else {
                    catalogue.AddBus(ParseName(request.AsMap()),
                                     ParseRoute(request.AsMap()),
                                     (ParseRouteIsRoundtrip(request.AsMap()) ? BusType::RoundTrip : BusType::Basic));
                }
            }
        }

        for (const auto &request : base_requests) {
            if (ParseType(request.AsMap()) != "Stop"s) {
                continue;
            }
            for (const auto &stop_pair : ParseStopDistances(request.AsMap())) {
                catalogue.SetPreciseDistance(ParseName(request.AsMap()), stop_pair.first, stop_pair.second);
            }
        }
    }

    json::Node JsonReader::GetStatJson(const RequestHandler &handler) const {
        Array statJson;

        for (const auto &request : GetStatRequests()) {
            const auto cur_type = ParseType(request.AsMap());
            if (cur_type == "Stop"s) {
                statJson.push_back(StopRequestFormat(handler, request.AsMap()));
            } else if (cur_type == "Bus"s) {
                statJson.push_back(BusRequestFormat(handler, request.AsMap()));
            } else {
                statJson.push_back(MapRequestFormat(handler, request.AsMap()));
            }
        }
        return {statJson};
    }

    renderer::MapSettings JsonReader::GetMapSettings() const {
        auto settings_dict = GetRenderSettings();
        auto bus_label_offset = settings_dict.at("bus_label_offset").AsArray();
        auto stop_label_offset = settings_dict.at("stop_label_offset").AsArray();
        auto underlayer_color = settings_dict.at("underlayer_color");
        vector<svg::Color> color_palette;
        for (const auto &el : settings_dict.at("color_palette").AsArray()) {
            color_palette.push_back(ParseColor(el));
        }

        return {
            settings_dict.at("width"s).AsDouble(),
            settings_dict.at("height"s).AsDouble(),
            settings_dict.at("padding"s).AsDouble(),
            settings_dict.at("line_width"s).AsDouble(),
            settings_dict.at("stop_radius"s).AsDouble(),
            settings_dict.at("bus_label_font_size"s).AsInt(),
            {bus_label_offset.front().AsDouble(), bus_label_offset.back().AsDouble()},
            settings_dict.at("stop_label_font_size"s).AsInt(),
            {stop_label_offset.front().AsDouble(), stop_label_offset.back().AsDouble()},
            ParseColor(underlayer_color),
            settings_dict.at("underlayer_width"s).AsDouble(),
            color_palette};
    }

    void Run(TransportCatalogue &catalogue, std::istream &in_stream, std::ostream &out_stream) {
        JsonReader reader(in_stream);
        reader.ApplyBaseRequests(catalogue);

        renderer::MapRenderer map_renderer(move(reader.GetMapSettings()), catalogue.GetWorkingStopsCoordinates());

        RequestHandler handler(catalogue, map_renderer);
        Print(Document(reader.GetStatJson(handler)), out_stream);
    }

} // namespace transport_catalogue::json_reader