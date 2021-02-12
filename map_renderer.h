#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <string>

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

struct renderSettings {
    double width = 0;
    double height = 0;

    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;

    double bus_label_font_size = 0;
    std::array<double,2> bus_label_offset = {0,0};
    // point

    double stop_label_font_size = 0;
    std::array<double,2> stop_label_offset = {0,0};

    svg::Color underlayer_color = {};
    double underlayer_width = 0;

    std::vector<svg::Color> color_palette = {};
};

class MapScaler {
    // В этой задаче используется модель плоской земли, поддерживаемой тремя слонами, стоящими на трех китах.
    // Параметр padding необходим для того, чтобы автобус не смог подъехать слишком близко к краю земли
    // и не упал в бескрайний океан, что привело бы к переполнению координат конечной остановки и выбросу исключения. )))
public:
    template <typename PointInputIt>
    MapScaler(PointInputIt points_begin, PointInputIt points_end, double max_width,
        double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lng < rhs.lng;
                });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lat < rhs.lat;
                });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!this->IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!this->IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(width_zoom.value(), height_zoom.value());
        }
        else if (width_zoom) {
            zoom_coeff_ = width_zoom.value();
        }
        else if (height_zoom) {
            zoom_coeff_ = height_zoom.value();
        }
    }

    bool IsZero(double value);

    svg::Point operator()(Coordinates coords) const {
        return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
    }

private:
    double padding_ = 0.0;
    double min_lon_ = 0.0;
    double max_lat_ = 0.0;
    double zoom_coeff_ = 0.0;
};


class map_renderer
{
public:
    map_renderer(transport::Catalogue& catalogue, const renderSettings& settings)
        :catalogue_(catalogue), settings_(settings) {}
    std::string render() const;
private:
    svg::Point coordinatesToPoint(const Coordinates& place) const;
    const transport::Catalogue& catalogue_;
    const renderSettings& settings_;
};

