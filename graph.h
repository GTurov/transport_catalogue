#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default;
    explicit DirectedWeightedGraph(size_t vertex_count);
    EdgeId addEdge(const Edge<Weight>& edge);

    size_t getVertexCount() const;
    size_t getEdgeCount() const;
    const Edge<Weight>& getEdge(EdgeId edge_id) const;
    IncidentEdgesRange getIncidentEdges(VertexId vertex) const;

private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidenceLists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertexCount)
    : incidenceLists_(vertexCount) {
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::addEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidenceLists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::getVertexCount() const {
    return incidenceLists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::getEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::getEdge(EdgeId edgeId) const {
    return edges_.at(edgeId);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::getIncidentEdges(VertexId vertex) const {
    return ranges::asRange(incidenceLists_.at(vertex));
}
}  // namespace graph
