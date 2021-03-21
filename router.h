#pragma once

#include "graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace graph {

template <typename Weight>
class Router {
private:
    using Graph = DirectedWeightedGraph<Weight>;

public:
    explicit Router(const Graph& graph);

    struct RouteInfo {
        Weight weight;
        std::vector<EdgeId> edges;
    };

    std::optional<RouteInfo> buildRoute(VertexId from, VertexId to) const;

private:
    struct RouteInternalData {
        Weight weight;
        std::optional<EdgeId> prevEdge;
    };
    using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

    void initializeRoutesInternalData(const Graph& graph) {
        const size_t vertexCount = graph.getVertexCount();
        for (VertexId vertex = 0; vertex < vertexCount; ++vertex) {
            routesInternalData_[vertex][vertex] = RouteInternalData{ZERO_WEIGHT, std::nullopt};
            for (const EdgeId edgeId : graph.getIncidentEdges(vertex)) {
                const auto& edge = graph.getEdge(edgeId);
                if (edge.weight < ZERO_WEIGHT) {
                    throw std::domain_error("Edges' weights should be non-negative");
                }
                auto& routeInternalData = routesInternalData_[vertex][edge.to];
                if (!routeInternalData || routeInternalData->weight > edge.weight) {
                    routeInternalData = RouteInternalData{edge.weight, edgeId};
                }
            }
        }
    }

    void relaxRoute(VertexId vertexFrom, VertexId vertexTo, const RouteInternalData& routeFrom,
                    const RouteInternalData& routeTo) {
        auto& routeRelaxing = routesInternalData_[vertexFrom][vertexTo];
        const Weight candidateWeight = routeFrom.weight + routeTo.weight;
        if (!routeRelaxing || candidateWeight < routeRelaxing->weight) {
            routeRelaxing = {candidateWeight,
                              routeTo.prevEdge ? routeTo.prevEdge : routeFrom.prevEdge};
        }
    }

    void relaxRoutesInternalDataThroughVertex(size_t vertexCount, VertexId vertexThrough) {
        for (VertexId vertexFrom = 0; vertexFrom < vertexCount; ++vertexFrom) {
            if (const auto& routeFrom = routesInternalData_[vertexFrom][vertexThrough]) {
                for (VertexId vertexTo = 0; vertexTo < vertexCount; ++vertexTo) {
                    if (const auto& routeTo = routesInternalData_[vertexThrough][vertexTo]) {
                        relaxRoute(vertexFrom, vertexTo, *routeFrom, *routeTo);
                    }
                }
            }
        }
    }

    static constexpr Weight ZERO_WEIGHT{};
    const Graph& graph_;
    RoutesInternalData routesInternalData_;
};

template <typename Weight>
Router<Weight>::Router(const Graph& graph)
    : graph_(graph)
    , routesInternalData_(graph.getVertexCount(),
                            std::vector<std::optional<RouteInternalData>>(graph.getVertexCount()))
{
    initializeRoutesInternalData(graph);

    const size_t vertexCount = graph.getVertexCount();
    for (VertexId vertexThrough = 0; vertexThrough < vertexCount; ++vertexThrough) {
        relaxRoutesInternalDataThroughVertex(vertexCount, vertexThrough);
    }
}

template <typename Weight>
std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::buildRoute(VertexId from,
                                                                             VertexId to) const {
    const auto& routeInternalData = routesInternalData_.at(from).at(to);
    if (!routeInternalData) {
        return std::nullopt;
    }
    const Weight weight = routeInternalData->weight;
    std::vector<EdgeId> edges;
    for (std::optional<EdgeId> edgeId = routeInternalData->prevEdge;
         edgeId;
         edgeId = routesInternalData_[from][graph_.getEdge(*edgeId).from]->prevEdge)
    {
        edges.push_back(*edgeId);
    }
    std::reverse(edges.begin(), edges.end());

    return RouteInfo{weight, std::move(edges)};
}

}  // namespace graph
