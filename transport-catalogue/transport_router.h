#pragma once

#include "router.h"
#include "transport_catalogue.h"

namespace transport_catalogue::router {

    struct RouterSettings {
        int bus_wait_time;
        double bus_velocity;
    };

    struct StopVertices {
        graph::VertexId wait_vertex;
        graph::VertexId bus_vertex;
    };

    class TransportRouter {
    public:
        TransportRouter(const TransportCatalogue &db, RouterSettings settings);
        std::optional<RouteInfo> BuildRoute(const Stop *from, const Stop *to) const;

    private:
        using Graph = graph::DirectedWeightedGraph<double>;
        using Router = graph::Router<double>;

        constexpr static double km_hour_to_m_minute = 1000.0 / 60.0;

        RouterSettings settings_;

        std::unordered_map<Stop const *, StopVertices> stop_to_vertices_;
        std::unordered_map<graph::EdgeId, RouteItem> edge_to_data_;

        Graph graph_;
        Router router_;

        Graph BuildGraph(const TransportCatalogue &db);
        void AddBusEdges(const TransportCatalogue &db, Graph &graph);
        double GetTravelTime(const double total_distance) const;
    };

} // namespace transport_catalogue::router