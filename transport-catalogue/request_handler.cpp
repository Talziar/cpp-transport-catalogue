#include "request_handler.h"

using namespace std;

namespace transport_catalogue {

    // ---------- RequestHandler ----------

    RequestHandler::RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer, const router::TransportRouter &router) : db_(db), renderer_(renderer), router_(router) {}

    optional<BusSet const *> RequestHandler::GetBusesByStop(const string_view &stop_name) const {
        return db_.GetBusesByStop(stop_name);
    }

    optional<BusInfo> RequestHandler::GetBusInfo(const string_view bus_name) const {
        return db_.GetBusInfo(bus_name);
    }

    void RequestHandler::RenderMap(ostream &out_stream) const {
        renderer_.Render(move(db_.GetSortedBuses()), move(db_.GetSortedStops())).Render(out_stream);
    }

    std::optional<RouteInfo> RequestHandler::GetRoute(const string_view from, const string_view to) const {
        return router_.BuildRoute(db_.FindStop(from), db_.FindStop(to));
    }

} // namespace transport_catalogue