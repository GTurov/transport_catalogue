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
    template <typename ForwardIt>
    MapScaler(ForwardIt pointsBegin, ForwardIt pointsEnd, double maxWidth,
        double maxHeight, double padding);
    bool isZero(double value) { return std::abs(value) < 1e-6;}
    svg::Point operator()(geo::Coordinates coords) const;
private:
    double computeZoom(std::pair<double, double> minMaxLat, std::pair<double, double> minMaxLon,
                       double maxWidth, double maxHeight);
    template <typename ForwardIt, typename Getter>
    static std::pair<double, double> minMaxField(ForwardIt first, ForwardIt last, Getter get);
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

    svg::Document renderRoutesLines(const transport::RouteSet& routes, const MapScaler& scaler) const;
    svg::Document renderRoutesLabels(const transport::RouteSet& routes, const MapScaler& scaler) const;
    svg::Document renderStopRounds(const transport::StopSet& stops, const MapScaler& scaler) const;
    svg::Document renderStopLabels(const transport::StopSet& stops, const MapScaler& scaler) const;
    svg::Polyline makeRouteLine(const transport::Route* route, const MapScaler& scaler, int colorIndex) const;
    svg::Text makeRouteLabelUnderlayer(svg::Point position, const std::string& name) const;
    svg::Text makeRouteLabel(svg::Point position, const std::string& name, int colorIndex) const;
    svg::Circle makeStopCircle(svg::Point position) const;
    svg::Text makeStopLabelUnderlayer(svg::Point position, const std::string& name) const;
    svg::Text makeStopLabel(svg::Point position, const std::string& name) const;
    const transport::Catalogue& catalogue_;
    const RenderSettings& settings_;
};

