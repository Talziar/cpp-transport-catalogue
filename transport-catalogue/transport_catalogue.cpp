#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue {

    void TransportCatalogue::AddStop(const string_view stop_name, const geo::Coordinates stop_coordinates) {
        stops_.emplace_back(string(stop_name), stop_coordinates);
        stopname_to_stop_.emplace(stops_.back().name_, &stops_.back());
        stopname_to_buses_[stops_.back().name_];
    }

    void TransportCatalogue::AddBus(const string_view bus_name, const vector<string_view> &bus_stops) {
        buses_.emplace_back(string(bus_name), vector<Stop *>{});

        for (auto stop : bus_stops) {
            stopname_to_buses_[stop].insert(&buses_.back());
            buses_.back().second.push_back(FindStop(stop));
        }

        busname_to_bus_.emplace(buses_.back().first, &buses_.back());
    }

    void TransportCatalogue::SetPreciseDistance(const std::string_view stop_from_name, const std::string_view stop_to_name, uint32_t distance) {
        stops_to_distance_[{FindStop(stop_from_name), FindStop(stop_to_name)}] = distance;
    }

    Stop *TransportCatalogue::FindStop(const string_view stop_name) const {
        auto found = stopname_to_stop_.find(stop_name);
        return (found == stopname_to_stop_.end() ? nullptr : found->second);
    }

    Bus *TransportCatalogue::FindBus(const string_view bus_name) const {
        auto found = busname_to_bus_.find(bus_name);
        return (found == busname_to_bus_.end() ? nullptr : found->second);
    }

    optional<set<Bus *, bus_compare> const *> TransportCatalogue::GetBusesByStop(const std::string_view stop_name) const {
        auto found = stopname_to_buses_.find(stop_name);
        if (found == stopname_to_buses_.end()) {
            return nullopt;
        }
        return &(found->second);
    }

    uint64_t TransportCatalogue::GetPreciseDistance(Stop const *from, Stop const *to) const {
        auto found = stops_to_distance_.find({from, to});
        if (found == nullptr) {
            found = stops_to_distance_.find({to, from});
        }
        return found->second;
    }

    BusInfo TransportCatalogue::GetBusInfo(const string_view bus_name) const {
        Bus *required_bus = FindBus(bus_name);
        if (required_bus == nullptr) {
            return BusInfo();
        }

        BusInfo required_bus_info;
        unordered_set<string_view> unique_stops;
        double geographical_distance = 0.0;

        required_bus_info.name_ = required_bus->first;
        required_bus_info.stop_count_ = required_bus->second.size();

        for (uint16_t i = 0; i + 1u < required_bus->second.size(); ++i) {
            if (i == 0) {
                unique_stops.insert(required_bus->second[i]->name_);
            }
            unique_stops.insert(required_bus->second[i + 1]->name_);

            required_bus_info.route_length_ += GetPreciseDistance(required_bus->second[i], required_bus->second[i + 1]);
            geographical_distance += geo::ComputeDistance(required_bus->second[i]->coordinates_, required_bus->second[i + 1]->coordinates_);
        }
        required_bus_info.unique_stop_count_ = unique_stops.size();
        required_bus_info.curvature_ = (required_bus_info.route_length_ * 1.0) / geographical_distance;

        return required_bus_info;
    }

} // namespace transport_catalogue
