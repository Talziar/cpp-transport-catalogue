#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace transport_catalogue::json_reader {

    class JsonReader {
    public:
        JsonReader(std::istream &in_stream) : jsonDocument_(json::Load(in_stream)) {}

        json::Array GetBaseRequests() const;
        json::Array GetStatRequests() const;
        json::Dict GetRenderSettings() const;

        void ApplyBaseRequests(TransportCatalogue &catalogue) const;
        json::Node GetStatJson(const RequestHandler &handler) const;
        renderer::MapSettings GetMapSettings() const;

    private:
        json::Document jsonDocument_;

        static std::string ParseType(const json::Dict &dict_node);
        static std::string ParseName(const json::Dict &dict_node);
        static geo::Coordinates ParseCoordinates(const json::Dict &dict_node);
        static bool ParseRouteIsRoundtrip(const json::Dict &dict_node);
        static std::vector<std::string> ParseRoute(const json::Dict &dict_node);
        static std::vector<std::pair<std::string, uint32_t>> ParseStopDistances(const json::Dict &dict_node);
        static svg::Color ParseColor(const json::Node &json_string_or_array);

        static json::Node StopRequestFormat(const RequestHandler &r_h, json::Dict stat_request);
        static json::Node BusRequestFormat(const RequestHandler &r_h, json::Dict stat_request);
        static json::Node MapRequestFormat(const RequestHandler &r_h, json::Dict stat_request);
    };

    void Run(TransportCatalogue &catalogue, std::istream &in_stream, std::ostream &out_stream);

} // namespace transport_catalogue::json_reader