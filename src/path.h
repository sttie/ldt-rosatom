#pragma once

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exterior_property.hpp>

#include "structs.h"

using EdgeWeightProperty = boost::property<boost::edge_weight_t, double>;
using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, EdgeWeightProperty>;

using Path = std::vector<VertID>; // set of vertices
using Routes = std::vector<std::vector<Path>>; // matrix of full path between every pair of vertices

using DistanceProperty = boost::exterior_vertex_property<Graph, double>;
using DistanceMatrix = DistanceProperty::matrix_type;

template <typename Graph>
inline auto GetEdgeWeight(
        const Graph& graph,
        typename boost::graph_traits<Graph>::vertex_descriptor v1,
        typename boost::graph_traits<Graph>::vertex_descriptor v2) {
    auto [edge, found] = boost::edge(v1, v2, graph);
    if (!found) {
        throw std::runtime_error("no such edge between " + std::to_string(v1) + " and " + std::to_string(v2));
    }

    return boost::get(boost::edge_weight_t(), graph, edge);
}

class PathManager {
private:
    Graph graph;
    Routes routes;
    std::unordered_map<BoatID, Voyage> current_voyage;

    DistanceMatrix distances;

public:
    std::shared_ptr<Icebreakers> icebreakers;
    std::shared_ptr<Ships> ships;
    PathManager(Graph graph, std::shared_ptr<Icebreakers> icebreakers, std::shared_ptr<Ships> ships);
    // build path to point, return next step, update current_route for all boats in caravan
    Voyage sail2point(Icebreaker &icebreaker, VertID point, Date current_time);
    // build path to all icebreaker's caravan final points, return next step, update current_route
    Voyage sail2depots(Icebreaker &icebreaker, Date current_time);
    Voyage getCurrentVoyage(BoatID boat);

    std::pair<VertID, double> GetNearestVertex(VertID source, const std::vector<VertID>& vertexes) const;
    double PathDistance(VertID start, const std::vector<VertID>& points) const;

private:
    VertID GetNextVertexInShortestPath(VertID current, VertID end) const;
    double GetMinimalSpeedInCaravan(const Icebreaker& icebreaker, const std::set<BoatID>& caravan) const;
};

