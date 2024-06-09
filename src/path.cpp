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
Voyage PathManager::sail2point(Icebreaker &icebreaker, VertID point, Date current_time) {
    auto next_vertex = GetNextVertexInShortestPath(icebreaker.cur_pos, point);
    
    Voyage voyage;
    voyage.start_time = current_time;
    voyage.start_point = icebreaker.cur_pos;
    voyage.end_time = current_time + GetEdgeWeight(graph, icebreaker.cur_pos, next_vertex) /
                                     GetMinimalSpeedInCaravan(icebreaker, icebreaker.caravan);
    voyage.end_point = next_vertex;

    return voyage;
}

// build path to all icebreaker's caravan final points, return next step
Voyage PathManager::sail2depots(Icebreaker &icebreaker, Date current_time) {
    std::vector<VertID> all_caravan_end_points;
    for (auto ship_id : icebreaker.caravan) {
        all_caravan_end_points.push_back((*ships)[ship_id].finish);
    }

    // тут должно быть оптимальное построение пути по всем точкам (задача коммивояжера), но пока здесь путь до первой попавшейся
    if (!all_caravan_end_points.empty()) {
        return sail2point(icebreaker, all_caravan_end_points.front(), current_time);
    }
    return {};
}

Voyage PathManager::getCurrentVoyage(BoatID boat) {
    if (current_voyage.count(boat))
        return current_voyage[boat];
    return {};
}

VertID PathManager::GetNextVertexInShortestPath(VertID current, VertID end) const {
    int optimal_neighbour, optimal_metric; bool found = false;

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

double PathManager::GetMinimalSpeedInCaravan(const Icebreaker& icebreaker, const std::set<BoatID>& caravan) const {
    double min_speed = icebreaker.knot_speed;
    for (auto ship_id : caravan) {
        min_speed = std::min(min_speed, (*ships)[ship_id].knot_speed);
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

double PathManager::PathDistance(VertID start, const std::vector<VertID>& points) const {
    if (points.empty()) {
        throw std::runtime_error("PathDistance(): unable to process empty vertexes vector");
    }

    double distance = distances[start][points.front()];
    for (size_t i = 1; i < points.size(); ++i) {
        distance += distances[points[i - 1]][points[i]];
    }

    return distance;
}
