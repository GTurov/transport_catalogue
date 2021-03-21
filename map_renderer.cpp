#include "map_renderer.h"

//#define SVG_TO_FILE

#ifdef SVG_TO_FILE
#include <fstream> // debug
#endif
#include <unordered_set>
#include <sstream>

std::string MapRenderer::render() const {
    using namespace std::literals;

    const transport::RouteSet routesToDraw = catalogue_.allRoutes();
    transport::StopSet stopsToDraw;
    for (transport::Route* r: routesToDraw) {
        for (transport::Stop* s: r->stops()) {
            stopsToDraw.insert(s);
        }
    }

    MapScaler scaler(stopsToDraw.begin(), stopsToDraw.end(),
                     settings_.width, settings_.height, settings_.padding);

    svg::Document svgDocument;

    // Lines
    int colorIndex = 0;
    for (transport::Route* r: routesToDraw) {
        if (r->stopsCount() < 2) {
            continue;
        }
        svg::Polyline routeLine = svg::Polyline()
                .setStrokeColor(settings_.colorPalette[colorIndex])
                .setStrokeWidth(settings_.lineWidth)
                .setFillColor({})
                .setStrokeLineCap(svg::StrokeLineCap::ROUND)
                .setStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                ;
        ++colorIndex;
        if (colorIndex == (int)settings_.colorPalette.size()) {
            colorIndex = 0;
        }
        for (auto it = r->stops().begin(); it != r->stops().end(); ++it) {
            routeLine.addPoint(scaler((*it)->place()));
        }
        if (!r->isCycled()) {
            for (auto rIt = ++r->stops().rbegin(); rIt != r->stops().rend(); ++rIt) {
                routeLine.addPoint(scaler((*rIt)->place()));
            }
        }
        svgDocument.add(routeLine);
    }

    // Route labels
    colorIndex = 0;
    for (transport::Route* r: routesToDraw) {
        if (r->stopsCount() == 0) {
            continue;
        }

        svg::Text routeLabelUnderlayer = svg::Text()
                .setPosition(scaler((*r->stops().begin())->place()))
                .setOffset(settings_.busLabelOffset)
                .setFontSize(settings_.busLabelFontSize)
                .setFontFamily("Verdana")
                .setFontWeight("bold"s)
                .setData(r->name())
                .setFillColor(settings_.underlayerColor)
                .setStrokeColor(settings_.underlayerColor)
                .setStrokeWidth(settings_.underlayerWidth)
                .setStrokeLineCap(svg::StrokeLineCap::ROUND)
                .setStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                ;
        svgDocument.add(routeLabelUnderlayer);

        svg::Text routeLabel = svg::Text()
                .setPosition(scaler((*r->stops().begin())->place()))
                .setOffset(settings_.busLabelOffset)
                .setFontSize(settings_.busLabelFontSize)
                .setFontFamily("Verdana")
                .setFontWeight("bold"s)
                .setData(r->name())
                .setFillColor(settings_.colorPalette[colorIndex])
                ;
        svgDocument.add(routeLabel);

        if (!r->isCycled() && r->stops().size()>1 && (*r->stops().begin() != *r->stops().rbegin())) {
            svg::Text stopLabelUnderlayer = svg::Text()
                    .setPosition(scaler((*r->stops().rbegin())->place()))
                    .setOffset(settings_.busLabelOffset)
                    .setFontSize(settings_.busLabelFontSize)
                    .setFontFamily("Verdana")
                    .setFontWeight("bold"s)
                    .setData(r->name())
                    .setFillColor(settings_.underlayerColor)
                    .setStrokeColor(settings_.underlayerColor)
                    .setStrokeWidth(settings_.underlayerWidth)
                    .setStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .setStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    ;
            svgDocument.add(stopLabelUnderlayer);

            svg::Text stopLabel = svg::Text()
                    .setPosition(scaler((*r->stops().rbegin())->place()))
                    .setOffset(settings_.busLabelOffset)
                    .setFontSize(settings_.busLabelFontSize)
                    .setFontFamily("Verdana")
                    .setFontWeight("bold"s)
                    .setData(r->name())
                    .setFillColor(settings_.colorPalette[colorIndex])
                    ;
            svgDocument.add(stopLabel);
        }
        ++colorIndex;
        if (colorIndex == (int)settings_.colorPalette.size()) {
            colorIndex = 0;
        }
    }

    // Stop rounds
    for (transport::Stop* s: stopsToDraw) {
        svg::Circle stopRound = svg::Circle()
                .setCenter(scaler(s->place()))
                .setRadius(settings_.stopRadius)
                .setFillColor("white"s)
                ;
        svgDocument.add(stopRound);
    }

    //Stop labels
    for (transport::Stop* s: stopsToDraw) {
        svg::Text stopLabelUnderlayer = svg::Text()
                .setPosition(scaler(s->place()))
                .setOffset(settings_.stopLabelOffset)
                .setFontSize(settings_.stopLabelFontSize)
                .setFontFamily("Verdana")
                .setData(s->name())
                .setFillColor(settings_.underlayerColor)
                .setStrokeColor(settings_.underlayerColor)
                .setStrokeWidth(settings_.underlayerWidth)
                .setStrokeLineCap(svg::StrokeLineCap::ROUND)
                .setStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                ;
        svgDocument.add(stopLabelUnderlayer);

        svg::Text stopLabel = svg::Text()
                .setPosition(scaler(s->place()))
                .setOffset(settings_.stopLabelOffset)
                .setFontSize(settings_.stopLabelFontSize)
                .setFontFamily("Verdana")
                .setData(s->name())
                .setFillColor("black")
                ;
        svgDocument.add(stopLabel);
    }

#ifdef SVG_TO_FILE
    std::ofstream svgFile("map.svg"s);
    svgDocument.Render(svgFile);
    svgFile.close();
#endif
    std::string buffer;
    std::stringstream ss(buffer);
    svgDocument.render(ss);

    return ss.str();
}
