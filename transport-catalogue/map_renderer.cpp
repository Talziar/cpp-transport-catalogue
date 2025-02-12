#include "map_renderer.h"

namespace transport_catalogue::renderer {

    using namespace std::literals;

    using namespace svg;

    // ---------- SphereProjector ----------

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

    // ---------- MapRenderer BusLine ----------

    void MapRenderer::RenderBusLine(const Bus *bus, Color cur_color, Document &document) const {
        using namespace svg;
        Polyline polyline;
        polyline.SetStrokeColor(cur_color).SetFillColor(NoneColor).SetStrokeWidth(settings_.line_width).SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);

        for (const auto &stop : bus->second) {
            polyline.AddPoint(projector_(stop->coordinates_));
        }
        document.Add(polyline);
    }

    void MapRenderer::RenderBusLines(const BusSet &buses, Document &document) const {
        int color_count = settings_.color_palette.size();
        int cur_index = 0;
        for (const Bus *bus : buses) {
            Color cur_color = settings_.color_palette[cur_index % color_count];
            RenderBusLine(bus, cur_color, document);
            cur_index++;
        }
    }

    // ---------- MapRenderer BusLabel ----------

    void MapRenderer::RenderBusLabel(const Bus *bus, Color cur_color, const Stop *end_stop, Document &document) const {
        using namespace svg;
        document.Add(Text().SetPosition(projector_(end_stop->coordinates_)).SetOffset({settings_.bus_label_offset}).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold").SetData(bus->first.bus_name).SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND));
        document.Add(Text().SetPosition(projector_(end_stop->coordinates_)).SetOffset({settings_.bus_label_offset}).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold").SetData(bus->first.bus_name).SetFillColor(cur_color));
    }

    void MapRenderer::RenderBusLabels(const BusSet &buses, Document &document) const {
        int color_count = settings_.color_palette.size();
        int cur_index = 0;
        for (const Bus *bus : buses) {
            Color cur_color = settings_.color_palette[cur_index % color_count];
            RenderBusLabel(bus, cur_color, bus->second.front(), document);
            if (bus->first.bus_type == BusType::Basic && bus->second.front() != bus->second[bus->second.size() / 2]) {
                RenderBusLabel(bus, cur_color, bus->second[bus->second.size() / 2], document);
            }
            cur_index++;
        }
    }

    // ---------- MapRenderer StopSign ----------

    void MapRenderer::RenderStopSign(const Stop *stop, Document &document) const {
        using namespace svg;
        document.Add(Circle().SetCenter(projector_(stop->coordinates_)).SetRadius(settings_.stop_radius).SetFillColor("white"s));
    }

    void MapRenderer::RenderStopSigns(const StopSet &stops, Document &document) const {
        for (const Stop *stop : stops) {
            RenderStopSign(stop, document);
        }
    }

    // ---------- MapRenderer StopLabel ----------

    void MapRenderer::RenderStopLabel(const Stop *stop, Document &document) const {
        using namespace svg;
        document.Add(Text().SetPosition(projector_(stop->coordinates_)).SetOffset({settings_.stop_label_offset}).SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s).SetData(stop->name_).SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND));
        document.Add(Text().SetPosition(projector_(stop->coordinates_)).SetOffset({settings_.stop_label_offset}).SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s).SetData(stop->name_).SetFillColor("black"));
    }

    void MapRenderer::RenderStopLabels(const StopSet &stops, Document &document) const {
        for (const Stop *stop : stops) {
            RenderStopLabel(stop, document);
        }
    }

    // ---------- MapRenderer ----------

    MapRenderer::MapRenderer(MapSettings &&settings, const std::deque<geo::Coordinates> &working_stops_coords) : settings_(std::move(settings)), projector_(working_stops_coords.begin(), working_stops_coords.end(), settings_.width, settings_.height, settings_.padding) {}

    Document MapRenderer::Render(const BusSet &&sorted_buses, const StopSet &&sorted_stops) const {
        Document document;
        RenderBusLines(sorted_buses, document);
        RenderBusLabels(sorted_buses, document);

        RenderStopSigns(sorted_stops, document);
        RenderStopLabels(sorted_stops, document);
        return document;
    }

} // namespace transport_catalogue::renderer
