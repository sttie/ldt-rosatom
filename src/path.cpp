#include "path.h"

#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <thread>
#include <mutex>

namespace {

DistanceMatrix DistanceMatrixByGraph(Graph& graph) {
    // std::cout << "start DistanceMatrixByGraph..." << std::endl;

    // using WeightMap = boost::property_map<Graph, boost::edge_weight_t>::type;
    using DistanceMatrixMap = DistanceProperty::matrix_map_type;

    auto weight_pmap = boost::get(&EdgeProperty::weight, graph);

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
    return static_cast<Days>(day % 7 + 1) + frac;
}

size_t GetIcebreakerIndexByName(const std::string& name) {
    return icebreaker_name_to_index.at(boost::algorithm::to_lower_copy(name));
}

void FloydWarshallThread(
        const std::string& date, std::array<Graph, GRAPH_CLASSES_AMOUNT>& ice_graphs_,
        std::mutex& date_to_distances_mutex, DatesToDistances& date_to_distances) {
    std::array<DistanceMatrix, GRAPH_CLASSES_AMOUNT> ice_distances_mtrx = {
        DistanceMatrixByGraph(ice_graphs_[0]),
        DistanceMatrixByGraph(ice_graphs_[1]),
        DistanceMatrixByGraph(ice_graphs_[2]),
        DistanceMatrixByGraph(ice_graphs_[3]),
        DistanceMatrixByGraph(ice_graphs_[4]),
        DistanceMatrixByGraph(ice_graphs_[5]),
        DistanceMatrixByGraph(ice_graphs_[6]),
        DistanceMatrixByGraph(ice_graphs_[7]),
        DistanceMatrixByGraph(ice_graphs_[8]),
        DistanceMatrixByGraph(ice_graphs_[9])
    };

    date_to_distances_mutex.lock();
    date_to_distances.insert({date, std::move(ice_distances_mtrx)});
    date_to_distances_mutex.unlock();
}

}

// возвращает пустой вектор, если пути В ОДИНОЧКУ не существует
std::vector<Voyage> PathManager::GetShortestPathAlone(const Ship& ship, VertID start, VertID end) const {
    std::vector<Voyage> path;

    size_t ship_index = ship_class_to_index.at(ship.ice_class);

    VertID current_vert = start;
    Days fake_time = cur_time;

    while (current_vert != end) {
        auto [next_vert, edge_ice_type] = GetNextVertexInShortestPathAlone(current_vert, ship, end);

        if (next_vert == -1) {
            return {};
        }

        Voyage current_voyage;
        current_voyage.start_point = current_vert;
        current_voyage.end_point = next_vert;
        current_voyage.start_time = fake_time;

        auto okay_date = GetCurrentOkayDateByTime(fake_time);
        const auto& graph = date_to_graph.at(okay_date).at(ship_index);
        float time_to_next_vert = GetEdgeWeight(graph, current_vert, next_vert) / ship.speed;
        current_voyage.end_time = fake_time + time_to_next_vert;

        fake_time = current_voyage.end_time;

        path.push_back(current_voyage);
    }

    return path;
}

// ВРЕМЯ ВЫДАЕТСЯ ТАК, КАК БУДТО ВСЕ ВРЕМЯ В ПУТИ КОРАБЛЬ ship БЫЛ КОРАБЛЕМ  С МИНИМАЛЬНОЙ СКОРОСТЬЮ
float PathManager::TimeToArriveUnderFakeProvodka(const Ship& ship, VertID start, VertID end) const {
    size_t ship_index = ship_class_to_index.at(ship.ice_class);

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    const auto& distance = date_to_distances.at(okay_date).at(ship_index);
    return distance[start][end] / ship.speed;
}

// время в пути корабля в одиночку
float PathManager::TimeToArriveAlone(const Ship& ship, VertID start, VertID end) const {
    size_t ship_index = ship_class_to_index.at(ship.ice_class);

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    const auto& graph = date_to_graph.at(okay_date).at(ship_index);

    VertID current_vert = start;
    float time = 0;

    while (current_vert != end) {
        auto [next_vert, edge_ice_type] = GetNextVertexInShortestPathAlone(current_vert, ship, end);

        if (next_vert == -1) {
            return std::numeric_limits<float>::infinity();
        }

        time += (GetEdgeWeight(graph, current_vert, next_vert) / ship.speed);
        current_vert = next_vert;
    }

    return time;
}

PathManager::PathManager(DatesToIceGraph date_to_graph_, std::shared_ptr<Icebreakers> icebreakers, std::shared_ptr<Ships> ships)
    : date_to_graph(std::move(date_to_graph_))
    , icebreakers(std::move(icebreakers))
    , ships(std::move(ships))
{
    std::vector<std::thread> threads;
    std::mutex date_to_distances_mutex;
    
    for (auto& [date, ice_graphs] : date_to_graph) {
        threads.push_back(std::thread{FloydWarshallThread,
                                      std::cref(date), std::ref(ice_graphs),
                                      std::ref(date_to_distances_mutex), std::ref(date_to_distances)});
    }

    for (auto& t : threads) {
        t.join();
    }
}

// build path to point, return next step
Voyage PathManager::sail2point(const Icebreaker &icebreaker, const Caravan &caravan, VertID point) {
    auto [next_vertex, _] = GetNextVertexInShortestPath(icebreaker.cur_pos, icebreaker, caravan, point);
    // std::cout << "!!! metric_to_vertex: " << metric_to_vertex << "cur: " << icebreaker.cur_pos << ", next: " << next_vertex << std::endl;

    size_t icebreaker_graph_index = GetIcebreakerIndexByName(icebreaker.name);

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& graph = date_to_graph[okay_date].at(icebreaker_graph_index);

    Voyage voyage;
    voyage.start_time = cur_time;
    voyage.start_point = icebreaker.cur_pos;
    // GetEdgeWeight возвращает для ледоколов ВРЕМЯ, поэтому все ок, ни на что делить не нужно
    if (caravan.ships_id.empty()) {
        // если караван пустой, скорость складывается только из веса ребер
        voyage.end_time = cur_time + GetEdgeWeight(graph, icebreaker.cur_pos, next_vertex);
    } else {
        auto [edge, _] = boost::edge(icebreaker.cur_pos, next_vertex, graph);
        auto [min_speed, min_speed_ship_id] = GetMinimalSpeedInCaravan(caravan, graph[edge].ice_type);

        // если самый медленный - ледокол
        if (min_speed_ship_id == -1) {
            voyage.end_time = cur_time + GetEdgeWeight(graph, icebreaker.cur_pos, next_vertex);
        }
        // значит, самый медленный - корабль в караване
        else {
            auto& min_speed_ship_graph = date_to_graph.at(okay_date).at(ship_class_to_index.at((*ships)[min_speed_ship_id].ice_class));

            // все дебафы уже учтены в графе, просто нужно отдельно учитывать, что корабль в некоторых случаях не может идти по ребру сам
            voyage.end_time = cur_time + GetEdgeWeight(min_speed_ship_graph, icebreaker.cur_pos, next_vertex) / min_speed;
        }
    }

    voyage.end_point = next_vertex;

    icebreaker_to_voyage[icebreaker.id] = voyage;
    for (auto &ship_id: caravan.ships_id)
        ship_to_voyage[ship_id] = voyage;

    return voyage;
}

// build path to all icebreaker's caravan final points, return next step
Voyage PathManager::sail2depots(const Icebreaker &icebreaker, const Caravan &caravan) {
    std::vector<VertID> all_caravan_end_points;
    for (auto ship_id : caravan.ships_id) {
        all_caravan_end_points.push_back((*ships)[ship_id.id].finish);
    }

    // тут должно быть оптимальное построение пути по всем точкам (задача коммивояжера), но пока здесь путь до первой попавшейся
    if (!all_caravan_end_points.empty()) {
        auto [next, new_dist] = GetNearestVertex(icebreaker.cur_pos, icebreaker, all_caravan_end_points);
        auto res = sail2point(icebreaker, caravan, next);
        icebreaker_to_voyage[icebreaker.id] = res;
        for (auto &ship_id: caravan.ships_id)
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

Voyage PathManager::getCurrentVoyage(const Caravan &caravan) {
    if (caravan.icebreaker_id && icebreaker_to_voyage.count(caravan.icebreaker_id->id))
        return icebreaker_to_voyage[caravan.icebreaker_id->id];
    if (!caravan.ships_id.empty() && ship_to_voyage.count(*caravan.ships_id.begin()))
        return ship_to_voyage[caravan.ships_id.begin()->id];
    return {};
}

std::pair<VertID, float> PathManager::GetNextVertexInShortestPath(VertID current,
        const Icebreaker& icebreaker, const Caravan& caravan, VertID end) const {
    int optimal_neighbour;
    float optimal_metric = std::numeric_limits<float>::infinity();
    bool found = false;

    // std::cout << "search for path from " << current << " to " << end << std::endl;

    size_t icebreaker_graph_index = GetIcebreakerIndexByName(icebreaker.name);

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& icebreaker_graph = date_to_graph.at(okay_date).at(icebreaker_graph_index);

    for (auto neighbour : boost::make_iterator_range(boost::out_edges(current, icebreaker_graph))) {
        int target = boost::target(neighbour, icebreaker_graph);

        float metric;
        // GetEdgeWeight возвращает для ледоколов ВРЕМЯ, поэтому все ок, ни на что делить не нужно
        if (caravan.ships_id.empty()) {
            // если караван пустой, скорость складывается только из весов ребер
            auto& icebreaker_distances = date_to_distances.at(okay_date).at(icebreaker_graph_index);
            metric = GetEdgeWeight(icebreaker_graph, current, target) + icebreaker_distances[target][end];
        } else {
            auto [edge, _] = boost::edge(current, target, icebreaker_graph);
            auto [minimal_caravan_speed, min_ship_id] = GetMinimalSpeedInCaravan(caravan, icebreaker_graph[edge].ice_type);

            if (min_ship_id == -1) {
                // значит самый медленный - ледокол, а для него уже посчитана метрика в edge.weight
                auto& icebreaker_distances = date_to_distances.at(okay_date).at(icebreaker_graph_index);
                metric = GetEdgeWeight(icebreaker_graph, current, target) + icebreaker_distances[target][end];
            } else {
                // значит самый медленный - один из кораблей, значит будем считать скорость каравана по нему
                auto& graph = date_to_graph.at(okay_date).at(ship_class_to_index.at((*ships)[min_ship_id].ice_class));
                auto& distances = date_to_distances.at(okay_date).at(ship_class_to_index.at((*ships)[min_ship_id].ice_class));

                // std::cout << "min_caravan_peed: " << minimal_caravan_speed << " and its ship.id: " << min_ship_id
                //           << ", from: " << current << ", to: " << target << ", end: " << end
                //           << ", edge_weight: " << GetEdgeWeight(graph, current, target)
                //           << ", distance: " << distances[target][end]
                //           << ", okay_date: " << okay_date
                //           << ", index_map: " << ship_class_to_index.at((*ships)[min_ship_id].ice_class)
                //           << std::endl;

                // все дебафы уже учтены в графе, просто нужно отдельно учитывать, что корабль в некоторых случаях не может идти по ребру сам
                metric = GetEdgeWeight(graph, current, target) / minimal_caravan_speed + distances[target][end] / minimal_caravan_speed;
            }
        }

        if (metric < optimal_metric) {
            optimal_neighbour = target;
            optimal_metric = metric;
            found = true;

            // std::cout << "new optimal: " << metric << " to " << optimal_neighbour << " from " << current
            //           << ", taken edge: " << boost::source(neighbour, icebreaker_graph) << std::endl;
        }
    }

    if (!found) {
        throw std::runtime_error("lol no optimal path from " + std::to_string(current) + " to "
                                    + std::to_string(end) + " for icebreaker " + std::to_string(icebreaker.id.id) + " caravan size "
                                    + std::to_string(caravan.ships_id.size()));
    }

    return std::make_pair(optimal_neighbour, optimal_metric);
}

std::pair<int, int> PathManager::GetNextVertexInShortestPathAlone(VertID current, const Ship& ship, VertID end) const {
    int optimal_neighbour = -1;
    float optimal_metric = 10000.0f;
    bool found = false;

    size_t alone_ship_graph_index = alone_ship_class_to_index.at(ship.ice_class);

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& graph = date_to_graph.at(okay_date).at(alone_ship_graph_index);
    auto& distances = date_to_distances.at(okay_date).at(alone_ship_graph_index);

    for (auto neighbour : boost::make_iterator_range(boost::out_edges(current, graph))) {
        int target = boost::target(neighbour, graph);

        auto [edge, _] = boost::edge(current, target, graph);
        float metric = GetEdgeWeight(graph, current, target) / ship.speed + distances[target][end] / ship.speed;

        if (metric < optimal_metric) {
            optimal_neighbour = target;
            optimal_metric = metric;
            found = true;
        }
    }

    if (!found) {
        return std::make_pair(-1, -1);
    }

    auto [edge, _] = boost::edge(current, optimal_neighbour, graph);
    return std::make_pair(optimal_neighbour, graph[edge].ice_type);
}

std::pair<float, int> PathManager::GetMinimalSpeedInCaravan(const Caravan& caravan, int edge_ice_type) const {
    float min_speed = (*icebreakers)[caravan.icebreaker_id->id].speed;
    int min_sheep_id = -1;

    for (auto ship_id : caravan.ships_id) {
        auto ship = (*ships)[ship_id.id];
        float speed = ship.speed;

        if (speed < min_speed) {
            min_speed = std::min(min_speed, speed);
            min_sheep_id = ship_id.id;
        }
    }

    return std::make_pair(min_speed, min_sheep_id);
}

std::pair<VertID, float> PathManager::GetNearestVertex(VertID source, const Icebreaker& icebreaker, const std::vector<VertID>& vertexes) const {
    if (vertexes.empty()) {
        throw std::runtime_error("GetNearestVertex(): unable to process empty vertexes vector");
    }

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& distances = date_to_distances.at(okay_date)[GetIcebreakerIndexByName(icebreaker.name)];

    VertID nearest = vertexes.front();
    for (size_t i = 1; i < vertexes.size(); ++i) {
        if (distances[source][vertexes[i]] < distances[source][nearest]) {
            nearest = vertexes[i];
        }
    }

    return std::make_pair(nearest, distances[source][nearest]);
}

std::pair<VertID, float> PathManager::GetNearestVertex(VertID source, const Ship& ship, const std::vector<VertID>& vertexes) const {
    if (vertexes.empty()) {
        throw std::runtime_error("GetNearestVertex(): unable to process empty vertexes vector");
    }

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& distances = date_to_distances.at(okay_date)[ship_class_to_index.at(ship.ice_class)];

    VertID nearest = vertexes.front();
    for (size_t i = 1; i < vertexes.size(); ++i) {
        if (distances[source][vertexes[i]] < distances[source][nearest]) {
            nearest = vertexes[i];
        }
    }

    return std::make_pair(nearest, distances[source][nearest]);
}

float PathManager::PathDistance(VertID start, const Icebreaker& icebreaker, std::vector<VertID> points) const {
    if (points.empty()) {
        throw std::runtime_error("PathDistance(): unable to process empty vertexes vector");
    }

    float distance = 0;
    VertID cur_point = start;
    for (size_t i = 0; i < points.size(); ++i) {
        auto [next, new_dist] = GetNearestVertex(cur_point, icebreaker, points);
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
        std::vector<std::string> splitted_date;
        boost::split(splitted_date, date_str, [](char c) { return c == '-'; });

        int day = std::stoi(splitted_date[0]);
        int month_days = months[splitted_date[1]];
        int year = std::stoi(splitted_date[2]);

        Days total_days = 2 * 365 + (year - 1900) * 365 + (30/*високосные года*/) + /*на самом деле считается с 1899 30 декабря*/1 + month_days + day;
        dates.push_back({total_days, date_str});
    }

    std::sort(dates.begin(), dates.end(), [](const auto& el, const auto& el2) { return el.first < el2.first; });

    for (size_t i = 0; i < dates.size() - 1; ++i) {
        if (time >= dates[i].first && time <= dates[i + 1].first) {
            if (GetWeekDay(time) < 3.5) {
                return dates[i].second;
            } else {
                return dates[i + 1].second;
            }
        }
    }
    
    // не нашли, значит time - либо раньше самой первой карты, либо позже самой последней
    if (time < dates.front().first) {
        return dates.front().second;
    } else if (time > dates.back().first) {
        return dates.back().second;
    }

    throw std::runtime_error("lol no okay date!");
}
