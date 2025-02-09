#pragma once

#include "geo.h"

#include <deque>
#include <optional>
#include <set>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport_catalogue {

    struct Stop {
        std::string name_;
        geo::Coordinates coordinates_;
    };

    using Bus = std::pair<std::string, std::vector<Stop *>>;

    struct BusInfo {
        explicit operator bool() const { return !(name_.empty()); }
        std::string_view name_;
        uint32_t stop_count_;
        uint32_t unique_stop_count_;
        uint64_t route_length_ = 0;
        double curvature_;
    };

    struct bus_compare {
        bool operator()(const Bus *lhs, const Bus *rhs) const {
            return std::greater<>()(rhs->first, lhs->first);
        }
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string_view stop_name, const geo::Coordinates stop_coordinates);
        void AddBus(const std::string_view bus_name, const std::vector<std::string_view> &bus_stops);
        void SetPreciseDistance(const std::string_view stop_from_name, const std::string_view stop_to_name, uint32_t distance);

        Stop *FindStop(const std::string_view stop_name) const;
        Bus *FindBus(const std::string_view bus_name) const;

        std::optional<std::set<Bus *, bus_compare> const *> GetBusesByStop(const std::string_view stop_name) const;
        uint64_t GetPreciseDistance(Stop const *from, Stop const *to) const;
        BusInfo GetBusInfo(const std::string_view bus_name) const;

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
        std::unordered_map<std::string_view, std::set<Bus *, bus_compare>> stopname_to_buses_;
        std::unordered_map<std::pair<Stop const *, Stop const *>, uint64_t, StopPairHasher> stops_to_distance_;

        std::unordered_map<std::string_view, Bus *> busname_to_bus_;
    };

} // namespace transport_catalogue
