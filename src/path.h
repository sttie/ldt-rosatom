#pragma once

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exterior_property.hpp>
#include <optional>

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

struct PDPPoint {
    VertID vertex;
    std::optional<ShipId> ship_id;

    bool operator<(const PDPPoint& other) const {
        return vertex < other.vertex;
    }

    bool operator==(const PDPPoint& other) const {
        return vertex == other.vertex &&
                ((ship_id.has_value() && other.ship_id.has_value() && ship_id.value() == other.ship_id.value()) ||
                 (!ship_id.has_value() && !other.ship_id.has_value()));
    }
};

class PathManager {
private:
    DatesToIceGraph date_to_graph;
    DatesToDistances date_to_distances;
    size_t MAX_SHIPS_IN_CARAVAN = 3;

public:
    Days cur_time = 0;
    std::shared_ptr<Icebreakers> icebreakers;
    std::shared_ptr<Ships> ships;
    std::unordered_map<ShipId, Voyage> ship_to_voyage;
    std::unordered_map<IcebreakerId, Voyage> icebreaker_to_voyage;

    size_t GetMaxShipsInCaravan() const {
        return MAX_SHIPS_IN_CARAVAN;
    }

    PathManager(DatesToIceGraph date_to_graph, std::shared_ptr<Icebreakers> icebreakers, std::shared_ptr<Ships> ships, size_t MAX_SHIPS_IN_CARAVAN = 3);
    // build path to point, return next step, update current_route for all boats in caravan
    Voyage sail2point(const Icebreaker &icebreaker, const Caravan &caravan, VertID point);
    // build path to all icebreaker's caravan final points, return next step, update current_route
    Voyage sail2depots(const Icebreaker &icebreaker, const Caravan &caravan);
    Voyage getCurrentVoyage(ShipId ship_id);
    Voyage getCurrentVoyage(IcebreakerId icebreaker_id);
    Voyage getCurrentVoyage(const Caravan &caravan);

    std::pair<VertID, float> GetNearestVertex(VertID source, const Icebreaker& icebreaker, const std::vector<VertID>& vertexes) const;
    std::pair<VertID, float> GetNearestVertex(VertID source, const std::vector<VertID>& vertexes) const;
    float PathDistance(VertID start, const Icebreaker& icebreaker, std::vector<VertID> points) const;

    float TimeToArriveUnderFakeProvodka(const Ship& ship, const Icebreaker &icebreaker, VertID start, VertID end);
    float TimeToArriveAlone(const Ship& ship, VertID start, VertID end);

    std::vector<Voyage> GetShortestPathAlone(const Ship& ship, VertID start, VertID end);

    std::pair<float, std::vector<PDPPoint>> TimeToSail(const Caravan& caravan);
    std::vector<Schedule> SailPath(const Icebreaker& icebreaker, const std::vector<PDPPoint>& points);

    std::string GetCurrentOkayDateByTime(Days time) const;

    bool HasEdge(const Ship& ship, bool alone, Days time, VertID from, VertID to) const {
        size_t graph_index;
        if (alone) {
            graph_index = alone_ship_class_to_index.at(ship.ice_class);
        } else {
            graph_index = ship_class_to_index.at(ship.ice_class);
        }

        auto okay_date = GetCurrentOkayDateByTime(time);
        const auto& graph = date_to_graph.at(okay_date).at(graph_index);
        return boost::edge(from, to, graph).second;
    }

    bool HasEdge(const Icebreaker& icebreaker, Days time, VertID from, VertID to) const;

private:
    std::optional<VertID> GetNextVertexInShortestPath(VertID current, const Icebreaker& icebreaker, const Caravan& caravan, VertID end) const;
    std::optional<VertID> GetNextVertexInShortestPathAlone(VertID current, const Ship& ship, VertID end) const;

    const Ship* GetMinimalSpeedShipInCaravan(const Caravan& caravan) const;
    // std::string GetCurrentOkayDateByTime(Days time) const;

    std::optional<VertID> FindNewAchievablePoint(const Ship& ship, VertID from, std::unordered_set<VertID>& visited);
    void FixFinishForWeakShips(const std::vector<int>& weak_ships, const Icebreaker& icebreaker);

    float ShipTimeToArrive(
        size_t graph_index, const float speed,
        VertID start, VertID end,
        const std::function<std::optional<VertID>(VertID)>& next_vert_callback);

    std::vector<Voyage> GetShortestPathForCaravan(const Caravan& caravan, VertID start, VertID end);
};
