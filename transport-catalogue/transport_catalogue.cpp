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

    Stop *TransportCatalogue::FindStop(const string_view stop_name) const {
        return (stopname_to_stop_.contains(stop_name) ? stopname_to_stop_.at(stop_name) : nullptr);
    }

    Bus *TransportCatalogue::FindBus(const string_view bus_name) const {
        return (busname_to_bus_.contains(bus_name) ? busname_to_bus_.at(bus_name) : nullptr);
    }

    optional<vector<Bus *>> TransportCatalogue::GetBusesByStop(const std::string_view stop_name) const {
        if (!stopname_to_buses_.contains(stop_name)) {
            return nullopt;
        }
        return vector(stopname_to_buses_.at(stop_name).begin(), stopname_to_buses_.at(stop_name).end());
    }

    BusInfo
    TransportCatalogue::GetBusInfo(const string_view bus_name) const {
        Bus *required_bus = FindBus(bus_name);
        if (required_bus == nullptr) {
            return BusInfo();
        }

        BusInfo required_bus_info;
        required_bus_info.name_ = required_bus->first;
        required_bus_info.stop_count_ = required_bus->second.size();
        unordered_set<string_view> unique_stops;

        for (uint16_t i = 0; i + 1u < required_bus->second.size(); ++i) {
            if (i == 0) {
                unique_stops.insert(required_bus->second[i]->name_);
            }
            unique_stops.insert(required_bus->second[i + 1]->name_);
            required_bus_info.route_length_ += geo::ComputeDistance(required_bus->second[i]->coordinates_, required_bus->second[i + 1]->coordinates_);
        }
        required_bus_info.unique_stop_count_ = unique_stops.size();

        return required_bus_info;
    }

} // namespace transport_catalogue
