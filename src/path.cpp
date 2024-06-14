#include "path.h"

#include <boost/graph/floyd_warshall_shortest.hpp>

#include <iostream>

namespace {

DistanceMatrix DistanceMatrixByGraph(Graph& graph) {
    // using WeightMap = boost::property_map<Graph, boost::edge_weight_t>::type;
    using DistanceMatrixMap = DistanceProperty::matrix_map_type;

    auto weight_pmap = boost::get(&EdgeProperty::len, graph);

    // set the distance matrix to receive the floyd warshall output
    DistanceMatrix distances(boost::num_vertices(graph));
    DistanceMatrixMap dm(distances, graph);

    // find all pairs shortest paths
    bool valid = floyd_warshall_all_pairs_shortest_paths(graph, dm, boost::weight_map(weight_pmap));

    if (!valid) {
        throw std::runtime_error("floyd-warshall algorithm has returned valid = false!");
    }

    return distances;
}

Days GetWeekDay(Days time) {
    int day = static_cast<int>(time);
    float frac = time - static_cast<Days>(day);
    std::cout << "got week day: " << static_cast<Days>(day % 7 + 1) + frac << std::endl;
    return static_cast<Days>(day % 7 + 1) + frac;
}

float GetSpeedCoefByClass(int ice_type, IceClass ship_class) {
    if (ice_type == 0) {
        return 1.0f;
    } else if (ice_type == 3) {
        return 0.0f;
    }

    switch (ship_class) {
    case IceClass::kNoIceClass:
    case IceClass::kArc1:
    case IceClass::kArc2:
    case IceClass::kArc3: {
        return 0; // либо движение запрещено совсем, либо запрещено без проводки
    }
    case IceClass::kArc4:
    case IceClass::kArc5:
    case IceClass::kArc6: {
        if (ice_type == 1) {
            return 0.8f;
        } else if (ice_type == 2) {
            return 0.7f;
        }
    }
    case IceClass::kArc7: {
        if (ice_type == 2) {
            return 0.6f;
        } else if (ice_type == 3) {
            return 0.15f;
        }
    }
    default: {
        throw std::runtime_error("invalid ship class");
    }
    }
}

float GetIcebreakerSpeed(int ice_type, std::string icebreaker_name) {
    if (ice_type == 0) {
        return 1.0f;
    } else if (ice_type == 3) {
        return 0.0f;
    }

    if (ice_type == 1) {
        if (icebreaker_name == "50 лет Победы" || icebreaker_name == "Ямал") {
            return 19.0f;
        } else if (icebreaker_name == "Вайгач" || icebreaker_name == "Таймыр") {
            return 19.0f * 0.9f;
        }
    } else if (ice_type == 2) {
        if (icebreaker_name == "50 лет Победы" || icebreaker_name == "Ямал") {
            return 14.0f;
        } else if (icebreaker_name == "Вайгач" || icebreaker_name == "Таймыр") {
            return 14.0f * 0.75f;
        }
    }

    throw std::runtime_error("invalid icebreaker: " + icebreaker_name);
}

}

PathManager::PathManager(DatesToGraph date_to_graph, std::shared_ptr<Icebreakers> icebreakers, std::shared_ptr<Ships> ships)
    : date_to_graph(std::move(date_to_graph))
    , icebreakers(std::move(icebreakers))
    , ships(std::move(ships))
{
    // distances = DistanceMatrixByGraph(graph);
    for (auto& [date, graph] : date_to_graph) {
        date_to_distances.insert({date, DistanceMatrixByGraph(graph)});
    }

    // c(i, k) + d(k, j)
}

// build path to point, return next step
Voyage PathManager::sail2point(const Icebreaker &icebreaker, VertID point) {
    auto next_vertex = GetNextVertexInShortestPath(icebreaker.cur_pos, icebreaker.ice_class, icebreaker.speed, point);
    
    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& graph = date_to_graph[okay_date];

    Voyage voyage;
    voyage.start_time = cur_time;
    voyage.start_point = icebreaker.cur_pos;
    voyage.end_time = cur_time + GetEdgeLen(graph, icebreaker.cur_pos, next_vertex) /
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

VertID PathManager::GetNextVertexInShortestPath(VertID current, IceClass ice_class, float speed, VertID end) const {
    float optimal_neighbour, optimal_metric = std::numeric_limits<float>::infinity();
    bool found = false;

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& graph = date_to_graph.at(okay_date);
    auto& distances = date_to_distances.at(okay_date);

    for (auto neighbour : boost::make_iterator_range(boost::out_edges(current, graph))) {
        int target = boost::target(neighbour, graph);

        // TODO: STUPID, TIME AND DISTANCE AT THE SAME TIME
        auto metric = GetEdgeLen(graph, current, target) /  + distances[target][end];
        
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

float PathManager::GetMinimalSpeedInCaravan(const Caravan& caravan) const {
    float min_speed = (*icebreakers)[caravan.icebreaker_id.id].speed;
    for (auto ship_id : caravan.ships_id) {
        min_speed = std::min(min_speed, (*ships)[ship_id.id].speed);
    }

    return min_speed;
}

std::pair<VertID, float> PathManager::GetNearestVertex(VertID source, const std::vector<VertID>& vertexes) const {
    if (vertexes.empty()) {
        throw std::runtime_error("GetNearestVertex(): unable to process empty vertexes vector");
    }

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& graph = date_to_graph.at(okay_date);
    auto& distances = date_to_distances.at(okay_date);

    VertID nearest = vertexes.front();
    for (size_t i = 1; i < vertexes.size(); ++i) {
        if (distances[source][vertexes[i]] < distances[source][nearest]) {
            nearest = vertexes[i];
        }
    }

    return std::make_pair(nearest, distances[source][nearest]);
}

float PathManager::PathDistance(VertID start, std::vector<VertID> points) const {
    if (points.empty()) {
        throw std::runtime_error("PathDistance(): unable to process empty vertexes vector");
    }

    float distance = 0;
    VertID cur_point = start;
    for (size_t i = 0; i < points.size(); ++i) {
        auto [next, new_dist] = GetNearestVertex(cur_point, points);
        points.erase(std::find(points.begin(), points.end(), next));
        distance += new_dist;
        cur_point = next;
    }

    return distance;
}

std::string PathManager::GetCurrentOkayDateByTime(Days time) const {
    static std::unordered_map<std::string, int> months {
        {"Mar", 31 + 28},
        {"Apr", 31 + 28 + 31},
        {"May", 31 + 28 + 31 + 30}
    };

    std::vector<std::pair<Days, std::string>> dates;
    for (const auto& [date_str, graph] : date_to_graph) {
        int day, year; char month[4];
        scanf(date_str.data(), "%d-%s-%d", &day, month, &year);
        int month_days = months[std::string{month}];
        Days total_days = 2 * 365 + (year - 1900) * 365 + (30/*високосные года*/) + /*на самом деле считается с 1899 30 декабря*/1 + month_days + day;
        dates.push_back({total_days, date_str});
    }

    for (size_t i = 0; i < dates.size() - 1; ++i) {
        if (time >= dates[i].first && time <= dates[i + 1].first) {
            if (GetWeekDay(time) < 3.5) {
                return dates[i].second;
            } else {
                return dates[i + 1].second;
            }
        }
    }

    throw std::runtime_error("lol no okay date!");
}
