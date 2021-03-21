#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <string>

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

struct RenderSettings {
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
    MapScaler(StopIt pointsBegin, StopIt pointsEnd, double maxWidth,
        double maxHeight, double padding)
        : padding_(padding) {
        if (pointsBegin == pointsEnd) {
            return;
        }

        const auto [leftIt, rightIt]
            = std::minmax_element(pointsBegin, pointsEnd, [](auto lhs, auto rhs) {
            return lhs->place().lng < rhs->place().lng;
                });
        minLon_ = (*leftIt)->place().lng;
        const double maxLon = (*rightIt)->place().lng;

        const auto [bottomIt, topIt]
            = std::minmax_element(pointsBegin, pointsEnd, [](auto lhs, auto rhs) {
            return lhs->place().lat < rhs->place().lat;
                });
        const double minLat = (*bottomIt)->place().lat;
        maxLat_ = (*topIt)->place().lat;

        std::optional<double> widthZoom;
        if (!this->isZero(maxLon - minLon_)) {
            widthZoom = (maxWidth - 2 * padding) / (maxLon - minLon_);
        }

        std::optional<double> heightZoom;
        if (!this->isZero(maxLat_ - minLat)) {
            heightZoom = (maxHeight - 2 * padding) / (maxLat_ - minLat);
        }

        if (widthZoom && heightZoom) {
            zoomCoeff_ = std::min(widthZoom.value(), heightZoom.value());
        }
        else if (widthZoom) {
            zoomCoeff_ = widthZoom.value();
        }
        else if (heightZoom) {
            zoomCoeff_ = heightZoom.value();
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
    MapRenderer(transport::Catalogue& catalogue, const RenderSettings& settings)
        :catalogue_(catalogue), settings_(settings) {}
    std::string render() const;
private:
    const transport::Catalogue& catalogue_;
    const RenderSettings& settings_;
};

