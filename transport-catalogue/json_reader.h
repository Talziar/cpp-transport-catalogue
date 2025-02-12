#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "json.h"
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
    };

    void Run(TransportCatalogue &catalogue, std::istream &in_stream, std::ostream &out_stream);

} // namespace transport_catalogue::json_reader