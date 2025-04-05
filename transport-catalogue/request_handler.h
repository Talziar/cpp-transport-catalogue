#pragma once

#include "map_renderer.h"
#include "transport_router.h"

namespace transport_catalogue {

    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer, const router::TransportRouter &router);

        std::optional<BusSet const *> GetBusesByStop(const std::string_view &stop_name) const;

        std::optional<BusInfo> GetBusInfo(const std::string_view bus_name) const;

        void RenderMap(std::ostream &out_stream) const;
        std::optional<RouteInfo> GetRoute(const std::string_view from, const std::string_view to) const;

    private:
        const TransportCatalogue &db_;
        const renderer::MapRenderer &renderer_;
        const router::TransportRouter &router_;
    };

} // namespace transport_catalogue
