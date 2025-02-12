#include "json_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std;

namespace transport_catalogue::json_reader {

    namespace detail {

        // ---------- Parsing Helpers----------

        string ParseType(const json::Dict &dict_node) {
            return dict_node.at("type"s).AsString();
        }

        string ParseName(const json::Dict &dict_node) {
            return dict_node.at("name"s).AsString();
        }

        geo::Coordinates ParseCoordinates(const json::Dict &dict_node) {
            return {dict_node.at("latitude"s).AsDouble(), dict_node.at("longitude"s).AsDouble()};
        }

        bool ParseRouteIsRoundtrip(const json::Dict &dict_node) {
            return dict_node.at("is_roundtrip"s).AsBool();
        }

        vector<string> ParseRoute(const json::Dict &dict_node) {
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

        vector<pair<string, uint32_t>> ParseStopDistances(const json::Dict &dict_node) {
            auto road_to_distances = dict_node.at("road_distances"s).AsMap();
            vector<pair<string, uint32_t>> road_distances;

            for (auto [stop_name, distance] : road_to_distances) {
                road_distances.emplace_back(stop_name, distance.AsInt());
            }
            return road_distances;
        }

        // ---------- Print Helpers----------

        json::Node StopRequestFormat(const RequestHandler &r_h, json::Dict stat_request) {
            using namespace json;
            const auto stop_name = stat_request.at("name"s).AsString();
            const auto stop_buses = r_h.GetBusesByStop(stop_name);

            if (!stop_buses.has_value()) {
                return {Dict{{"request_id"s, stat_request.at("id"s)}, {"error_message"s, "not found"s}}};
            } else {
                Array stop_buses_vector;
                for (const auto el : *stop_buses.value()) {
                    stop_buses_vector.emplace_back(el->first.bus_name);
                }

                return {Dict{{"request_id"s, stat_request.at("id"s)}, {"buses"s, stop_buses_vector}}};
            }
        }

        json::Node BusRequestFormat(const RequestHandler &r_h, json::Dict stat_request) {
            using namespace json;
            const auto bus_name = stat_request.at("name"s).AsString();
            const auto req_bus_info = r_h.GetBusInfo(bus_name);

            if (!req_bus_info) {
                return {Dict{{"request_id"s, stat_request.at("id"s)}, {"error_message"s, "not found"s}}};
            } else {
                return {
                    Dict{
                        {"curvature"s, Node(req_bus_info->curvature_)},
                        {"request_id"s, stat_request.at("id"s)},
                        {"route_length"s, Node(int(req_bus_info->route_length_))},
                        {"stop_count"s, Node(int(req_bus_info->stop_count_))},
                        {"unique_stop_count"s, Node(int(req_bus_info->unique_stop_count_))}}};
            }
        }

        json::Node MapRequestFormat(const RequestHandler &r_h, json::Dict stat_request) {
            using namespace json;
            ostringstream s_stream;
            r_h.RenderMap(s_stream);

            return {
                Dict{
                    {"request_id"s, stat_request.at("id"s)},
                    {"map"s, s_stream.str()},
                }};
        }

        // ---------- Renderer Helpers----------

        svg::Color ParseColor(const json::Node &json_string_or_array) {
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

    } // namespace detail

    // ---------- JsonReader ----------

    json::Array JsonReader::GetBaseRequests() const {
        return jsonDocument_.GetRoot().AsMap().at("base_requests").AsArray();
    }

    json::Array JsonReader::GetStatRequests() const {
        return jsonDocument_.GetRoot().AsMap().at("stat_requests").AsArray();
    }

    json::Dict JsonReader::GetRenderSettings() const {
        return jsonDocument_.GetRoot().AsMap().at("render_settings").AsMap();
    }

    void JsonReader::ApplyBaseRequests(TransportCatalogue &catalogue) const {
        using namespace detail;
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
        using namespace detail;
        using namespace json;
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
        using namespace detail;
        using namespace json;

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

        renderer::MapRenderer map_renderer(move(reader.GetMapSettings()), transport_catalogue::detail::GetWorkingStopsCoordinates(catalogue));

        RequestHandler handler(catalogue, map_renderer);
        json::Print(json::Document(reader.GetStatJson(handler)), out_stream);
    }

} // namespace transport_catalogue::json_reader