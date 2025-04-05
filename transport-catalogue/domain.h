#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace transport_catalogue {

    struct Stop {
        std::string name_;
        geo::Coordinates coordinates_;
    };

    struct StopCompare {
        bool operator()(const Stop *lhs, const Stop *rhs) const;
    };

    using StopSet = std::set<Stop const *, StopCompare>;

    enum class BusType {
        NullType,
        Basic,
        RoundTrip
    };

    struct BusData {
        std::string bus_name = "";
        BusType bus_type = BusType::NullType;
    };

    using Bus = std::pair<BusData, std::vector<Stop const *>>;

    struct BusInfo {
        explicit operator bool() const;
        std::string_view name_;
        uint32_t stop_count_;
        uint32_t unique_stop_count_;
        uint64_t route_length_ = 0;
        double curvature_;
    };

    struct BusCompare {
        bool operator()(const Bus *lhs, const Bus *rhs) const;
    };

    using BusSet = std::set<Bus const *, BusCompare>;

    struct StopPairHasher {
        size_t operator()(const std::pair<Stop const *, Stop const *> &stop_ptr_pair) const {
            size_t h_first = stop_ptr_hasher_(stop_ptr_pair.first);
            size_t h_second = stop_ptr_hasher_(stop_ptr_pair.second);

            return h_first + h_second * 2027;
        }

    private:
        std::hash<Stop const *> stop_ptr_hasher_;
    };

    struct WaitItem {
        std::string stop_name;
        double time;
    };

    struct BusItem {
        std::string bus;
        int span_count;
        double time;
    };

    using RouteItem = std::variant<WaitItem, BusItem>;

    struct RouteInfo {
        double total_time;
        std::vector<RouteItem> items;
    };
}