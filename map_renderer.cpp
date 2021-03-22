#include "map_renderer.h"

//#define SVG_TO_FILE

#ifdef SVG_TO_FILE
#include <fstream> // debug
#endif
#include <unordered_set>
#include <sstream>

using namespace std::literals;

template <typename ForwardIt>
MapScaler::MapScaler(ForwardIt pointsBegin, ForwardIt pointsEnd, double maxWidth,
    double maxHeight, double padding)
    : padding_(padding) {
    if (pointsBegin == pointsEnd) {
        return;
    }

    auto minMaxLat = minMaxField(pointsBegin, pointsEnd, [](auto object) {
        return object->place().lat;
    });
    maxLat_ = minMaxLat.second;

    auto minMaxLon = minMaxField(pointsBegin, pointsEnd, [](auto object) {
        return object->place().lng;
    });
    minLon_ = minMaxLon.first;

    zoomCoeff_ = computeZoom(minMaxLat, minMaxLon, maxWidth, maxHeight);
}

svg::Point MapScaler::operator()(geo::Coordinates coords) const {
    return { (coords.lng - minLon_) * zoomCoeff_ + padding_,
            (maxLat_ - coords.lat) * zoomCoeff_ + padding_ };
}

double MapScaler::computeZoom(std::pair<double, double> minMaxLat, std::pair<double, double> minMaxLon,
                   double maxWidth, double maxHeight) {
    std::optional<double> widthZoom;
    if (!this->isZero(minMaxLon.second - minMaxLon.first)) {
        widthZoom = (maxWidth - 2 * padding_) / (minMaxLon.second - minMaxLon.first);
    }

    std::optional<double> heightZoom;
    if (!this->isZero(minMaxLat.second - minMaxLat.first)) {
        heightZoom = (maxHeight - 2 * padding_) / (minMaxLat.second - minMaxLat.first);
    }
    if (widthZoom && heightZoom) {
        return std::min(widthZoom.value(), heightZoom.value());
    }
    else if (widthZoom) {
        return widthZoom.value();
    }
    else if (heightZoom) {
        return heightZoom.value();
    } else {
        return 0.0;
    }
}

template <typename ForwardIt, typename Getter>
std::pair<double, double> MapScaler::minMaxField(ForwardIt first, ForwardIt last, Getter get) {
    const auto [leftIt, rightIt]
        = std::minmax_element(first, last, [&get](auto lhs, auto rhs) {
        return get(lhs) < get(rhs);
            });
    double minLon = get(*leftIt);
    double maxLon = get(*rightIt);
    return {minLon, maxLon};
}

std::string MapRenderer::render() const {
    const transport::RouteSet routesToDraw = catalogue_.allRoutes();
    const transport::StopSet stopsToDraw = [&routesToDraw](){
        transport::StopSet result;
        for (transport::Route* route: routesToDraw) {
            for (transport::Stop* s: route->stops()) {
                result.insert(s);
            }
        }
        return result;
    }();

    MapScaler scaler(stopsToDraw.begin(), stopsToDraw.end(),
                     settings_.width, settings_.height, settings_.padding);

    svg::Document svgDocument;
    svgDocument.combine(renderRoutesLines(routesToDraw, scaler));
    svgDocument.combine(renderRoutesLabels(routesToDraw, scaler));
    svgDocument.combine(renderStopRounds(stopsToDraw, scaler));
    svgDocument.combine(renderStopLabels(stopsToDraw, scaler));

#ifdef SVG_TO_FILE
    std::ofstream svgFile("map.svg"s);
    svgDocument.render(svgFile);
    svgFile.close();
#endif
    std::string buffer;
    std::stringstream ss(buffer);
    svgDocument.render(ss);

    return ss.str();
}

svg::Document MapRenderer::renderRoutesLines(const transport::RouteSet& routes, const MapScaler& scaler) const {
    svg::Document svgDocument;
    int colorIndex = 0;
    for (transport::Route* route: routes) {
        if (route->stopsCount() < 2) {
            continue;
        }
        svgDocument.add(makeRouteLine(route, scaler, colorIndex));
        ++colorIndex;
        if (colorIndex == (int)settings_.colorPalette.size()) {
            colorIndex = 0;
        }
    }
    return svgDocument;
}

svg::Document MapRenderer::renderRoutesLabels(const transport::RouteSet& routes, const MapScaler& scaler) const {
    svg::Document svgDocument;
    int colorIndex = 0;
    for (transport::Route* route: routes) {
        if (route->stopsCount() == 0) {
            continue;
        }
        svgDocument.add(makeRouteLabelUnderlayer(scaler((*route->stops().begin())->place()), route->name()));
        svgDocument.add(makeRouteLabel(scaler((*route->stops().begin())->place()), route->name(), colorIndex));
        if (!route->isCycled() && route->stops().size()>1 && (*route->stops().begin() != *route->stops().rbegin())) {
            svgDocument.add(makeRouteLabelUnderlayer(scaler((*route->stops().rbegin())->place()), route->name()));
            svgDocument.add(makeRouteLabel(scaler((*route->stops().rbegin())->place()), route->name(), colorIndex));
        }
        ++colorIndex;
        if (colorIndex == (int)settings_.colorPalette.size()) {
            colorIndex = 0;
        }
    }
    return svgDocument;
}

svg::Document MapRenderer::renderStopRounds(const transport::StopSet& stops, const MapScaler& scaler) const {
    svg::Document svgDocument;
    for (transport::Stop* stop: stops) {
        svgDocument.add(makeStopCircle(scaler(stop->place())));
    }
    return svgDocument;
}

svg::Document MapRenderer::renderStopLabels(const transport::StopSet& stops, const MapScaler& scaler) const {
    svg::Document svgDocument;
    for (transport::Stop* stop: stops) {
        svgDocument.add(makeStopLabelUnderlayer(scaler(stop->place()),stop->name()));
        svgDocument.add(makeStopLabel(scaler(stop->place()),stop->name()));
    }
    return svgDocument;
}

svg::Polyline MapRenderer::makeRouteLine(const transport::Route* route, const MapScaler& scaler, int colorIndex) const {
    svg::Polyline routeLine = svg::Polyline()
            .setStrokeColor(settings_.colorPalette[colorIndex])
            .setStrokeWidth(settings_.lineWidth)
            .setFillColor({})
            .setStrokeLineCap(svg::StrokeLineCap::ROUND)
            .setStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            ;
    for (auto it = route->stops().begin(); it != route->stops().end(); ++it) {
        routeLine.addPoint(scaler((*it)->place()));
    }
    if (!route->isCycled()) {
        for (auto rIt = ++route->stops().rbegin(); rIt != route->stops().rend(); ++rIt) {
            routeLine.addPoint(scaler((*rIt)->place()));
        }
    }
    return routeLine;
}

svg::Text MapRenderer::makeRouteLabelUnderlayer(svg::Point position, const std::string& name) const {
    return svg::Text()
            .setPosition(position)
            .setOffset(settings_.busLabelOffset)
            .setFontSize(settings_.busLabelFontSize)
            .setFontFamily("Verdana")
            .setFontWeight("bold"s)
            .setData(name)
            .setFillColor(settings_.underlayerColor)
            .setStrokeColor(settings_.underlayerColor)
            .setStrokeWidth(settings_.underlayerWidth)
            .setStrokeLineCap(svg::StrokeLineCap::ROUND)
            .setStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            ;
}

svg::Text MapRenderer::makeRouteLabel(svg::Point position, const std::string& name, int colorIndex) const {
    return svg::Text()
            .setPosition(position)
            .setOffset(settings_.busLabelOffset)
            .setFontSize(settings_.busLabelFontSize)
            .setFontFamily("Verdana")
            .setFontWeight("bold"s)
            .setData(name)
            .setFillColor(settings_.colorPalette[colorIndex])
            ;
}

svg::Circle MapRenderer::makeStopCircle(svg::Point position) const {
    return svg::Circle()
            .setCenter(position)
            .setRadius(settings_.stopRadius)
            .setFillColor("white"s)
            ;
}

svg::Text MapRenderer::makeStopLabelUnderlayer(svg::Point position, const std::string& name) const {
    return svg::Text()
            .setPosition(position)
            .setOffset(settings_.stopLabelOffset)
            .setFontSize(settings_.stopLabelFontSize)
            .setFontFamily("Verdana")
            .setData(name)
            .setFillColor(settings_.underlayerColor)
            .setStrokeColor(settings_.underlayerColor)
            .setStrokeWidth(settings_.underlayerWidth)
            .setStrokeLineCap(svg::StrokeLineCap::ROUND)
            .setStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            ;
}

svg::Text MapRenderer::makeStopLabel(svg::Point position, const std::string& name) const {
    return svg::Text()
            .setPosition(position)
            .setOffset(settings_.stopLabelOffset)
            .setFontSize(settings_.stopLabelFontSize)
            .setFontFamily("Verdana")
            .setData(name)
            .setFillColor("black")
            ;
}
