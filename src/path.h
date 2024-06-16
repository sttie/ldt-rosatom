#pragma once

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exterior_property.hpp>

#include "structs.h"

using EdgeWeightProperty = boost::property<boost::edge_weight_t, float>;

struct VertexProperty {
    float lat, lon;
    std::string name = "";
};

struct EdgeProperty {
    size_t start_id, end_id;
    int ice_type;
    float len;

    float weight;
};

// using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, EdgeWeightProperty>;

using Graph = boost::adjacency_list<boost::vecS,
                                    boost::vecS,
                                    boost::undirectedS,
                                    VertexProperty,
                                    EdgeProperty>;

using Path = std::vector<VertID>; // set of vertices
using Routes = std::vector<std::vector<Path>>; // matrix of full path between every pair of vertices

using DistanceProperty = boost::exterior_vertex_property<Graph, float>;
using DistanceMatrix = DistanceProperty::matrix_type;

const std::unordered_map<std::string, size_t> icebreaker_name_to_index = {
    {"50 лет Победы", 3},
    {"Ямал", 4},
    {"Вайгач", 5},
    {"Таймыр", 6}
};

const std::unordered_map<IceClass, size_t> ship_class_to_index = {
    {IceClass::kNoIceClass, 0},
    {IceClass::kArc1, 0},
    {IceClass::kArc2, 0},
    {IceClass::kArc3, 0},

    {IceClass::kArc4, 1},
    {IceClass::kArc5, 1},
    {IceClass::kArc6, 1},

    {IceClass::kArc7, 2},
};

const std::unordered_map<IceClass, size_t> alone_ship_class_to_index = {
    {IceClass::kNoIceClass, 7},
    {IceClass::kArc1, 7},
    {IceClass::kArc2, 7},
    {IceClass::kArc3, 7},

    {IceClass::kArc4, 8},
    {IceClass::kArc5, 8},
    {IceClass::kArc6, 8},

    {IceClass::kArc7, 9},
};

constexpr size_t GRAPH_CLASSES_AMOUNT = 11;

using DatesToIceGraph = std::unordered_map<std::string, std::array<Graph, GRAPH_CLASSES_AMOUNT>>;
using DatesToDistances = std::unordered_map<std::string, std::array<DistanceMatrix, GRAPH_CLASSES_AMOUNT>>;

inline auto GetEdgeWeight(
        const Graph& graph,
        typename boost::graph_traits<Graph>::vertex_descriptor v1,
        typename boost::graph_traits<Graph>::vertex_descriptor v2) {
    auto [edge, found] = boost::edge(v1, v2, graph);
    if (!found) {
        throw std::runtime_error("no such edge between " + std::to_string(v1) + " and " + std::to_string(v2));
    }

    return graph[edge].weight;
}

inline auto GetEdgeLen(
        const Graph& graph,
        typename boost::graph_traits<Graph>::vertex_descriptor v1,
        typename boost::graph_traits<Graph>::vertex_descriptor v2) {
    auto [edge, found] = boost::edge(v1, v2, graph);
    if (!found) {
        throw std::runtime_error("no such edge between " + std::to_string(v1) + " and " + std::to_string(v2));
    }

    return graph[edge].len;
}

class PathManager {
private:
    DatesToIceGraph date_to_graph;
    DatesToDistances date_to_distances;

public:
    Days cur_time = 0;
    std::shared_ptr<Icebreakers> icebreakers;
    std::shared_ptr<Ships> ships;
    std::unordered_map<ShipId, Voyage> ship_to_voyage;
    std::unordered_map<IcebreakerId, Voyage> icebreaker_to_voyage;

    PathManager(DatesToIceGraph date_to_graph, std::shared_ptr<Icebreakers> icebreakers, std::shared_ptr<Ships> ships);
    // build path to point, return next step, update current_route for all boats in caravan
    Voyage sail2point(const Icebreaker &icebreaker, const Caravan &caravan, VertID point);
    // build path to all icebreaker's caravan final points, return next step, update current_route
    Voyage sail2depots(const Icebreaker &icebreaker, const Caravan &caravan);
    Voyage getCurrentVoyage(ShipId ship_id);
    Voyage getCurrentVoyage(IcebreakerId icebreaker_id);
    Voyage getCurrentVoyage(const Caravan &caravan);

    std::pair<VertID, float> GetNearestVertex(VertID source, const Icebreaker& icebreaker, const std::vector<VertID>& vertexes) const;
    std::pair<VertID, float> GetNearestVertex(VertID source, const Ship& ship, const std::vector<VertID>& vertexes) const;
    float PathDistance(VertID start, const Icebreaker& icebreaker, std::vector<VertID> points) const;

    float TimeToArriveUnderFakeProvodka(const Ship& ship, VertID start, VertID end);
    float TimeToArriveAlone(const Ship& ship, VertID start, VertID end);

    std::vector<Voyage> GetShortestPathAlone(const Ship& ship, VertID start, VertID end);

private:
    std::pair<VertID, float> GetNextVertexInShortestPath(VertID current, const Icebreaker& icebreaker, const Caravan& caravan, VertID end) const;
    std::pair<int, int> GetNextVertexInShortestPathAlone(VertID current, const Ship& ship, VertID end) const;

    std::pair<float, int> GetMinimalSpeedInCaravan(const Caravan& caravan, int edge_ice_type) const;
    std::string GetCurrentOkayDateByTime(Days time) const;
};
