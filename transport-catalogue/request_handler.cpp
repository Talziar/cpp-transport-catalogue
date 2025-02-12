#include "request_handler.h"

using namespace std;

namespace transport_catalogue {

    // ---------- RequestHandler ----------

    RequestHandler::RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer) : db_(db), renderer_(renderer) {}

    optional<BusSet const *> RequestHandler::GetBusesByStop(const string_view &stop_name) const {
        return db_.GetBusesByStop(stop_name);
    }

    optional<BusInfo> RequestHandler::GetBusInfo(const string_view bus_name) const {
        Bus const *required_bus = db_.FindBus(bus_name);
        if (required_bus == nullptr) {
            return nullopt;
        }

        BusInfo required_bus_info;
        unordered_set<string_view> unique_stops;
        double geographical_distance = 0.0;

        required_bus_info.name_ = required_bus->first.bus_name;
        required_bus_info.stop_count_ = required_bus->second.size();

        for (uint16_t i = 0; i + 1u < required_bus->second.size(); ++i) {
            if (i == 0) {
                unique_stops.insert(required_bus->second[i]->name_);
            }
            unique_stops.insert(required_bus->second[i + 1]->name_);

            required_bus_info.route_length_ += db_.GetPreciseDistance(required_bus->second[i], required_bus->second[i + 1]);
            geographical_distance += geo::ComputeDistance(required_bus->second[i]->coordinates_, required_bus->second[i + 1]->coordinates_);
        }
        required_bus_info.unique_stop_count_ = unique_stops.size();
        required_bus_info.curvature_ = (required_bus_info.route_length_ * 1.0) / geographical_distance;

        return required_bus_info;
    }

    const BusSet RequestHandler::GetSortedBuses() const {
        BusSet sorted_buses;
        for (const auto &bus : db_.GetBuses()) {
            if (!bus.second.empty()) {
                sorted_buses.insert(&bus);
            }
        }
        return sorted_buses;
    }

    const StopSet RequestHandler::GetSortedStops() const {
        StopSet sorted_buses;
        for (const auto &stop : db_.GetStops()) {
            auto buses = db_.GetBusesByStop(stop.name_);
            if (buses && !buses.value()->empty()) {
                sorted_buses.insert(&stop);
            }
        }
        return sorted_buses;
    }

    void RequestHandler::RenderMap(ostream &out_stream) const {
        using namespace svg;
        Document document;

        const auto sorted_buses = GetSortedBuses();
        renderer_.RenderBusLines(sorted_buses, document);
        renderer_.RenderBusLabels(sorted_buses, document);

        const auto sorted_stops = GetSortedStops();
        renderer_.RenderStopSigns(sorted_stops, document);
        renderer_.RenderStopLabels(sorted_stops, document);

        document.Render(out_stream);
    }

} // namespace transport_catalogue