#include "transport_router.h"

using namespace std;

namespace transport_catalogue::router {
    TransportRouter::TransportRouter(const TransportCatalogue &db, RouterSettings settings) : settings_(settings), graph_(BuildGraph(db)), router_(graph_) {}

    optional<RouteInfo> TransportRouter::BuildRoute(const Stop *from, const Stop *to) const {
        using namespace graph;

        const VertexId from_vertex = stop_to_vertices_.at(from).wait_vertex;
        const VertexId to_vertex = stop_to_vertices_.at(to).wait_vertex;

        const auto route = router_.BuildRoute(from_vertex, to_vertex);
        if (!route) {
            return nullopt;
        }

        RouteInfo result;
        result.total_time = route->weight;
        for (const EdgeId edge_id : route->edges) {
            result.items.push_back(edge_to_data_.at(edge_id));
        }

        return result;
    }

    TransportRouter::Graph TransportRouter::BuildGraph(const TransportCatalogue &db) {
        using namespace graph;

        const double wait_time = static_cast<double>(settings_.bus_wait_time);

        const auto &stops = db.GetStops();
        Graph graph(stops.size() * 2);

        VertexId next_vertex_id = 0;
        for (const auto &stop : stops) {
            const VertexId wait_vertex = next_vertex_id++;
            const VertexId ride_vertex = next_vertex_id++;

            stop_to_vertices_[&stop] = StopVertices{wait_vertex, ride_vertex};

            const EdgeId new_edge = graph.AddEdge({wait_vertex, ride_vertex, wait_time});

            edge_to_data_[new_edge] = WaitItem{stop.name_, wait_time};
        }

        AddBusEdges(db, graph);

        return graph;
    }

    void TransportRouter::AddBusEdges(const TransportCatalogue &db, Graph &graph) {
        using namespace graph;
        for (const auto &bus : db.GetBuses()) {
            const string &bus_name = bus.first.bus_name;

            const auto &stops = bus.second;
            const size_t stop_count = stops.size();

            for (size_t i = 0; i < stop_count - 1; ++i) {
                double total_distance = 0.0;
                const Stop *from = stops[i];

                for (size_t j = i + 1; j < stop_count; ++j) {
                    const Stop *to = stops[j];

                    total_distance += db.GetPreciseDistance(stops[j - 1], to);
                    const double travel_time = GetTravelTime(total_distance);

                    const EdgeId edge_id = graph.AddEdge({stop_to_vertices_.at(from).bus_vertex,
                                                          stop_to_vertices_.at(to).wait_vertex,
                                                          travel_time});

                    edge_to_data_[edge_id] = BusItem{bus_name, static_cast<int>(j - i), travel_time};
                }
            }
        }
    }

    double TransportRouter::GetTravelTime(const double total_distance) const {
        return total_distance / (settings_.bus_velocity * km_hour_to_m_minute);
    }

} // namespace transport_catalogue::router