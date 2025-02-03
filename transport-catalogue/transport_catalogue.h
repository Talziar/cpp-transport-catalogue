#pragma once

#include "geo.h"

#include <deque>
#include <optional>
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
        double route_length_;
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string_view stop_name, const geo::Coordinates stop_coordinates);
        void AddBus(const std::string_view bus_name, const std::vector<std::string_view> &bus_stops);

        Stop *FindStop(const std::string_view stop_name) const;
        Bus *FindBus(const std::string_view bus_name) const;

        std::optional<std::vector<Bus *>> GetBusesByStop(const std::string_view stop_name) const;
        BusInfo GetBusInfo(const std::string_view bus_name) const;

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string_view, Stop *> stopname_to_stop_;
        std::unordered_map<std::string_view, std::unordered_set<Bus *>> stopname_to_buses_;
        std::unordered_map<std::string_view, Bus *> busname_to_bus_;
    };

} // namespace transport_catalogue
