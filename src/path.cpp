#include "path.h"

#include <boost/graph/floyd_warshall_shortest.hpp>

namespace {

DistanceMatrix DistanceMatrixByGraph(Graph& graph) {
    using WeightMap = boost::property_map<Graph, boost::edge_weight_t>::type;
    using DistanceMatrixMap = DistanceProperty::matrix_map_type;

    WeightMap weight_pmap = boost::get(boost::edge_weight, graph);

    // set the distance matrix to receive the floyd warshall output
    DistanceMatrix distances(boost::num_vertices(graph));
    DistanceMatrixMap dm(distances, graph);

    // find all pairs shortest paths
    bool valid = floyd_warshall_all_pairs_shortest_paths(graph, dm, 
                                                boost::weight_map(weight_pmap));

    if (!valid) {
        throw std::runtime_error("floyd-warshall algorithm has returned valid = false!");
    }

    return distances;
}

}

PathManager::PathManager(Graph graph, std::shared_ptr<Icebreakers> icebreakers, std::shared_ptr<Ships> ships)
    : graph(std::move(graph))
    , icebreakers(std::move(icebreakers))
    , ships(std::move(ships))
    , distances(boost::num_vertices(graph))
{
    distances = DistanceMatrixByGraph(graph);
    // c(i, k) + d(k, j)
}

// build path to point, return next step
Voyage PathManager::sail2point(const Icebreaker &icebreaker, VertID point) {
    auto next_vertex = GetNextVertexInShortestPath(icebreaker.cur_pos, point);
    
    Voyage voyage;
    voyage.start_time = cur_time;
    voyage.start_point = icebreaker.cur_pos;
    voyage.end_time = cur_time + GetEdgeWeight(graph, icebreaker.cur_pos, next_vertex) /
                                     (24 * GetMinimalSpeedInCaravan(icebreaker.caravan));
    voyage.end_point = next_vertex;

    icebreaker_to_voyage[icebreaker.id] = voyage;
    for (auto &ship_id: icebreaker.caravan.ships_id)
        ship_to_voyage[ship_id] = voyage;

    return voyage;
}

// build path to all icebreaker's caravan final points, return next step
Voyage PathManager::sail2depots(const Icebreaker &icebreaker) {
    std::vector<VertID> all_caravan_end_points;
    for (auto ship_id : icebreaker.caravan.ships_id) {
        all_caravan_end_points.push_back((*ships)[ship_id.id].finish);
    }

    // тут должно быть оптимальное построение пути по всем точкам (задача коммивояжера), но пока здесь путь до первой попавшейся
    if (!all_caravan_end_points.empty()) {
        auto [next, new_dist] = GetNearestVertex(icebreaker.cur_pos, all_caravan_end_points);
        auto res = sail2point(icebreaker, next);
        icebreaker_to_voyage[icebreaker.id] = res;
        for (auto &ship_id: icebreaker.caravan.ships_id)
            ship_to_voyage[ship_id] = res;
        return res;
    }
    return {};
}

Voyage PathManager::getCurrentVoyage(ShipId ship_id) {
    if (ship_to_voyage.count(ship_id))
        return ship_to_voyage[ship_id];
    return {};
}

Voyage PathManager::getCurrentVoyage(IcebreakerId icebreaker_id) {
    if (icebreaker_to_voyage.count(icebreaker_id))
        return icebreaker_to_voyage[icebreaker_id];
    return {};
}

VertID PathManager::GetNextVertexInShortestPath(VertID current, VertID end) const {
    float optimal_neighbour, optimal_metric = std::numeric_limits<float>::infinity();
    bool found = false;

    for (auto neighbour : boost::make_iterator_range(boost::out_edges(current, graph))) {
        int target = boost::target(neighbour, graph);
        auto metric = GetEdgeWeight(graph, current, target) + distances[target][end];
        
        if (!found || metric < optimal_metric) {
            optimal_neighbour = target;
            optimal_metric = metric;
            found = true;
        }
    }

    if (!found) {
        throw std::runtime_error("lol no optimal path from " + std::to_string(current) + " to " + std::to_string(end));
    }

    return optimal_neighbour;
}

double PathManager::GetMinimalSpeedInCaravan(const Caravan& caravan) const {
    double min_speed = (*icebreakers)[caravan.icebreaker_id.id].knot_speed;
    for (auto ship_id : caravan.ships_id) {
        min_speed = std::min(min_speed, (*ships)[ship_id.id].knot_speed);
    }

    return min_speed;
}

std::pair<VertID, double> PathManager::GetNearestVertex(VertID source, const std::vector<VertID>& vertexes) const {
    if (vertexes.empty()) {
        throw std::runtime_error("GetNearestVertex(): unable to process empty vertexes vector");
    }

    VertID nearest = vertexes.front();
    for (size_t i = 1; i < vertexes.size(); ++i) {
        if (distances[source][vertexes[i]] < distances[source][nearest]) {
            nearest = vertexes[i];
        }
    }

    return std::make_pair(nearest, distances[source][nearest]);
}

double PathManager::PathDistance(VertID start, std::vector<VertID> points) const {
    if (points.empty()) {
        throw std::runtime_error("PathDistance(): unable to process empty vertexes vector");
    }

    double distance = 0;
    VertID cur_point = start;
    for (size_t i = 0; i < points.size(); ++i) {
        auto [next, new_dist] = GetNearestVertex(cur_point, points);
        points.erase(std::find(points.begin(), points.end(), next));
        distance += new_dist;
        cur_point = next;
    }

    return distance;
}
