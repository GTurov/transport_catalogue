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
    double lineWidth = 0;
    double stopRadius = 0;

    int busLabelFontSize = 0;
    svg::Point busLabelOffset = {0,0};

    int stopLabelFontSize = 0;
    svg::Point stopLabelOffset = {0,0};

    svg::Color underlayerColor = {};
    double underlayerWidth = 0;

    std::vector<svg::Color> colorPalette = {};

};

class MapScaler {
    // В отличие от модуля geo.h здесь используется модель плоской земли, поддерживаемой тремя слонами, стоящими
    // на трех китах. Параметр padding необходим для того, чтобы автобус не смог подъехать слишком близко к краю земли
    // и не упал в бескрайний океан, что привело бы к переполнению координат конечной остановки и неопределённому поведению. )))
public:
    template <typename StopIt>
    MapScaler(StopIt points_begin, StopIt points_end, double max_width,
        double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs->place().lng < rhs->place().lng;
                });
        minLon_ = (*left_it)->place().lng;
        const double max_lon = (*right_it)->place().lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs->place().lat < rhs->place().lat;
                });
        const double min_lat = (*bottom_it)->place().lat;
        maxLat_ = (*top_it)->place().lat;

        std::optional<double> width_zoom;
        if (!this->isZero(max_lon - minLon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - minLon_);
        }

        std::optional<double> height_zoom;
        if (!this->isZero(maxLat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (maxLat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoomCoeff_ = std::min(width_zoom.value(), height_zoom.value());
        }
        else if (width_zoom) {
            zoomCoeff_ = width_zoom.value();
        }
        else if (height_zoom) {
            zoomCoeff_ = height_zoom.value();
        }
    }

    bool isZero(double value) { return std::abs(value) < 1e-6;}

    svg::Point operator()(geo::Coordinates coords) const {
        return {(coords.lng - minLon_) * zoomCoeff_ + padding_,
                (maxLat_ - coords.lat) * zoomCoeff_ + padding_ };
    }

private:
    double padding_ = 0.0;
    double minLon_ = 0.0;
    double maxLat_ = 0.0;
    double zoomCoeff_ = 0.0;
};


class MapRenderer {
public:
    MapRenderer(transport::Catalogue& catalogue, const renderSettings& settings)
        :catalogue_(catalogue), settings_(settings) {}
    std::string render() const;
private:
    const transport::Catalogue& catalogue_;
    const renderSettings& settings_;
};

