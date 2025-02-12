#include "domain.h"

namespace transport_catalogue {

    // ---------- Stop ----------

    bool StopCompare::operator()(const Stop *lhs, const Stop *rhs) const {
        return std::greater<>()(rhs->name_, lhs->name_);
    }

    // ---------- Bus ----------

    BusInfo::operator bool() const { return !(name_.empty()); }

    bool BusCompare::operator()(const Bus *lhs, const Bus *rhs) const {
        return std::greater<>()(rhs->first.bus_name, lhs->first.bus_name);
    }
}