#pragma once

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exterior_property.hpp>

#include "structs.h"

using EdgeWeightProperty = boost::property<boost::edge_weight_t, double>;

struct VertexProperty {
    float lat, lon;
    std::string name = "";
};

struct EdgeProperty {
    size_t start_id, end_id;
    double len;
    int ice_type;
};

// using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, EdgeWeightProperty>;

using Graph = boost::adjacency_list<boost::vecS,
                                    boost::vecS,
                                    boost::undirectedS,
                                    VertexProperty,
                                    EdgeProperty>;

using Path = std::vector<VertID>; // set of vertices
using Routes = std::vector<std::vector<Path>>; // matrix of full path between every pair of vertices

using DistanceProperty = boost::exterior_vertex_property<Graph, double>;
using DistanceMatrix = DistanceProperty::matrix_type;

using DatesToGraph = std::unordered_map<std::string, Graph>;
using DatesToDistances = std::unordered_map<std::string, DistanceMatrix>;

template <typename Graph>
inline auto GetEdgeWeight(
        const Graph& graph,
        typename boost::graph_traits<Graph>::vertex_descriptor v1,
        typename boost::graph_traits<Graph>::vertex_descriptor v2) {
    auto [edge, found] = boost::edge(v1, v2, graph);
    if (!found) {
        throw std::runtime_error("no such edge between " + std::to_string(v1) + " and " + std::to_string(v2));
    }

    return graph[edge].len;
    // return boost::get(boost::edge_weight_t(), graph, edge);
}

class PathManager {
private:
    DatesToGraph date_to_graph;
    Routes routes;

    std::unordered_map<ShipId, Voyage> ship_to_voyage;
    std::unordered_map<IcebreakerId, Voyage> icebreaker_to_voyage;

    DatesToDistances date_to_distances;

public:
    Days cur_time = 0;
    std::shared_ptr<Icebreakers> icebreakers;
    std::shared_ptr<Ships> ships;
    PathManager(DatesToGraph date_to_graph, std::shared_ptr<Icebreakers> icebreakers, std::shared_ptr<Ships> ships);
    // build path to point, return next step, update current_route for all boats in caravan
    Voyage sail2point(const Icebreaker &icebreaker, VertID point);
    // build path to all icebreaker's caravan final points, return next step, update current_route
    Voyage sail2depots(const Icebreaker &icebreaker);
    Voyage getCurrentVoyage(ShipId ship_id);
    Voyage getCurrentVoyage(IcebreakerId icebreaker_id);

    std::pair<VertID, double> GetNearestVertex(VertID source, const std::vector<VertID>& vertexes) const;
    double PathDistance(VertID start, std::vector<VertID> points) const;

private:
    VertID GetNextVertexInShortestPath(VertID current, VertID end) const;
    double GetMinimalSpeedInCaravan(const Caravan& caravan) const;
};

