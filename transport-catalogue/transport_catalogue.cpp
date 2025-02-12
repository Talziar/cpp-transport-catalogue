#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue {

    // ---------- TransportCatalogue ----------

    void TransportCatalogue::AddStop(const string_view stop_name, const geo::Coordinates stop_coordinates) {
        stops_.emplace_back(string(stop_name), stop_coordinates);
        stopname_to_stop_.emplace(stops_.back().name_, &stops_.back());
        stopname_to_buses_[stops_.back().name_];
    }

    void TransportCatalogue::AddBus(const string_view bus_name, const vector<std::string> &bus_stops, const BusType bus_type) {
        buses_.emplace_back(BusData(string(bus_name), bus_type), vector<Stop const *>{});

        for (auto stop : bus_stops) {
            stopname_to_buses_[stop].insert(&buses_.back());
            buses_.back().second.push_back(FindStop(stop));
        }

        busname_to_bus_.emplace(buses_.back().first.bus_name, &buses_.back());
    }

    void TransportCatalogue::SetPreciseDistance(const std::string_view stop_from_name, const std::string_view stop_to_name, uint32_t distance) {
        stops_to_distance_[{FindStop(stop_from_name), FindStop(stop_to_name)}] = distance;
    }

    Stop const *TransportCatalogue::FindStop(const string_view stop_name) const {
        auto found = stopname_to_stop_.find(stop_name);
        return (found == stopname_to_stop_.end() ? nullptr : found->second);
    }

    Bus const *TransportCatalogue::FindBus(const string_view bus_name) const {
        auto found = busname_to_bus_.find(bus_name);
        return (found == busname_to_bus_.end() ? nullptr : found->second);
    }

    optional<BusSet const *> TransportCatalogue::GetBusesByStop(const std::string_view stop_name) const {
        auto found = stopname_to_buses_.find(stop_name);
        if (found == stopname_to_buses_.end()) {
            return nullopt;
        }
        return &(found->second);
    }

    const std::deque<Stop> &TransportCatalogue::GetStops() const {
        return stops_;
    }

    const std::deque<Bus> &TransportCatalogue::GetBuses() const {
        return buses_;
    }

    uint64_t TransportCatalogue::GetPreciseDistance(Stop const *from, Stop const *to) const {
        auto found = stops_to_distance_.find({from, to});
        if (found == nullptr) {
            found = stops_to_distance_.find({to, from});
        }
        return found->second;
    }

} // namespace transport_catalogue
