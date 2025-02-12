#pragma once

#include "domain.h"

#include <deque>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace transport_catalogue {

    class TransportCatalogue {
    public:
        void AddStop(const std::string_view stop_name, const geo::Coordinates stop_coordinates);
        void AddBus(const std::string_view bus_name, const std::vector<std::string> &bus_stops, const BusType bus_type);

        void SetPreciseDistance(const std::string_view stop_from_name, const std::string_view stop_to_name, uint32_t distance);
        uint64_t GetPreciseDistance(Stop const *from, Stop const *to) const;

        Stop const *FindStop(const std::string_view stop_name) const;
        Bus const *FindBus(const std::string_view bus_name) const;

        const std::deque<Stop> &GetStops() const;
        const StopSet GetSortedStops() const;
        const std::deque<geo::Coordinates> GetWorkingStopsCoordinates() const;

        const std::deque<Bus> &GetBuses() const;
        const BusSet GetSortedBuses() const;

        std::optional<BusSet const *> GetBusesByStop(const std::string_view stop_name) const;
        std::optional<BusInfo> GetBusInfo(const std::string_view bus_name) const;

    private:
        struct StopPairHasher {
            size_t operator()(const std::pair<Stop const *, Stop const *> &stop_ptr_pair) const {
                size_t h_first = stop_ptr_hasher_(stop_ptr_pair.first);
                size_t h_second = stop_ptr_hasher_(stop_ptr_pair.second);

                return h_first + h_second * 2027;
            }

        private:
            std::hash<Stop const *> stop_ptr_hasher_;
        };

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string_view, Stop *> stopname_to_stop_;
        std::unordered_map<std::string_view, BusSet> stopname_to_buses_;
        std::unordered_map<std::pair<Stop const *, Stop const *>, uint64_t, StopPairHasher> stops_to_distance_;

        std::unordered_map<std::string_view, Bus const *> busname_to_bus_;
    };

} // namespace transport_catalogue
