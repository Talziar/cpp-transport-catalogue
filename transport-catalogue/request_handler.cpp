#include "request_handler.h"

using namespace std;

namespace transport_catalogue {

    // ---------- RequestHandler ----------

    RequestHandler::RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer) : db_(db), renderer_(renderer) {}

    optional<BusSet const *> RequestHandler::GetBusesByStop(const string_view &stop_name) const {
        return db_.GetBusesByStop(stop_name);
    }

    optional<BusInfo> RequestHandler::GetBusInfo(const string_view bus_name) const {
        return db_.GetBusInfo(bus_name);
    }

    void RequestHandler::RenderMap(ostream &out_stream) const {
        renderer_.Render(move(db_.GetSortedBuses()), move(db_.GetSortedStops())).Render(out_stream);
    }

} // namespace transport_catalogue