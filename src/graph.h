#pragma once

#include <boost/graph/graph_traits.hpp>
#include <string>

template <typename Graph>
auto GetEdgeWeight(
        const Graph& graph,
        typename boost::graph_traits<Graph>::vertex_descriptor v1,
        typename boost::graph_traits<Graph>::vertex_descriptor v2) {
    auto [edge, found] = boost::edge(v1, v2, graph);
    if (!found) {
        throw std::runtime_error("no such edge between " + std::to_string(v1) + " and " + std::to_string(v2));
    }

    return boost::get(boost::edge_weight_t(), graph, edge);
}
