#include "path.h"

#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <thread>
#include <mutex>
#include <stack>

#include "util.h"

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

DistanceMatrix LenDistanceMatrixByGraph(Graph& graph) {
    // std::cout << "start DistanceMatrixByGraph..." << std::endl;

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
        DistanceMatrixByGraph(ice_graphs_[9]),
        LenDistanceMatrixByGraph(ice_graphs_[10])
    };

    date_to_distances_mutex.lock();
    date_to_distances.insert({date, std::move(ice_distances_mtrx)});
    date_to_distances_mutex.unlock();
}

}

float PathManager::ShipTimeToArrive(
        size_t graph_index, const float speed,
        VertID start, VertID end,
        const std::function<std::optional<VertID>(VertID)>& next_vert_callback)
{
    VertID current_vert = start;
    float time_to_arrive = 0;

    auto old_cur_time = cur_time;

    while (current_vert != end) {
        auto next_vert = next_vert_callback(current_vert);

        if (!next_vert.has_value()) {
            return std::numeric_limits<float>::infinity();
        }

        auto okay_date = GetCurrentOkayDateByTime(cur_time);
        const auto& graph = date_to_graph.at(okay_date).at(graph_index);
        auto time_to_next_vert = GetEdgeWeight(graph, current_vert, next_vert.value()) / speed;

        cur_time += time_to_next_vert;
        time_to_arrive += time_to_next_vert;
        current_vert = next_vert.value();
    }

    cur_time = old_cur_time;

    return time_to_arrive;
}

// возвращает пустой вектор, если пути В ОДИНОЧКУ не существует
std::vector<Voyage> PathManager::GetShortestPathAlone(const Ship& ship, VertID start, VertID end) {
    std::vector<Voyage> path;

    size_t ship_index = alone_ship_class_to_index.at(ship.ice_class);

    VertID current_vert = start;
    Days fake_time = cur_time;

    while (current_vert != end) {
        auto old_cur_time = cur_time;
        cur_time = fake_time;
        auto next_vert = GetNextVertexInShortestPathAlone(current_vert, ship, end);
        cur_time = old_cur_time;

        if (!next_vert.has_value()) {
            return {};
        }

        Voyage current_voyage;
        current_voyage.start_point = current_vert;
        current_voyage.end_point = next_vert.value();
        current_voyage.start_time = fake_time;

        auto okay_date = GetCurrentOkayDateByTime(fake_time);
        const auto& graph = date_to_graph.at(okay_date).at(ship_index);
        float time_to_next_vert = GetEdgeWeight(graph, current_vert, next_vert.value()) / ship.speed;
        current_voyage.end_time = fake_time + time_to_next_vert;

        fake_time = current_voyage.end_time;

        path.push_back(current_voyage);

        current_vert = next_vert.value();
    }

    return path;
}

// ВРЕМЯ ВЫДАЕТСЯ ТАК, КАК БУДТО ВСЕ ВРЕМЯ В ПУТИ КОРАБЛЬ ship БЫЛ КОРАБЛЕМ  С МИНИМАЛЬНОЙ СКОРОСТЬЮ
float PathManager::TimeToArriveUnderFakeProvodka(const Ship& ship, const Icebreaker &icebreaker, VertID start, VertID end) {
    size_t ship_index = ship_class_to_index.at(ship.ice_class);

    Caravan fake_caravan;
    fake_caravan.icebreaker_id = icebreaker.id;
    fake_caravan.ships_id.insert(ship.id);

    return ShipTimeToArrive(
        ship_index, ship.speed,
        start, end,
        [this, &icebreaker, &fake_caravan, end](VertID from) {
            return GetNextVertexInShortestPath(from, icebreaker, fake_caravan, end);
        }
    );
}

// время в пути корабля в одиночку
float PathManager::TimeToArriveAlone(const Ship& ship, VertID start, VertID end) {
    size_t ship_index = alone_ship_class_to_index.at(ship.ice_class);
    
    return ShipTimeToArrive(
        ship_index, ship.speed,
        start, end,
        [this, &ship, end](VertID from) {
            return GetNextVertexInShortestPathAlone(from, ship, end);
        }
    );
}

PathManager::PathManager(DatesToIceGraph date_to_graph_, std::shared_ptr<Icebreakers> icebreakers, std::shared_ptr<Ships> ships,
                         size_t MAX_SHIPS_IN_CARAVAN)
    : date_to_graph(std::move(date_to_graph_))
    , icebreakers(std::move(icebreakers))
    , ships(std::move(ships))
    , MAX_SHIPS_IN_CARAVAN(MAX_SHIPS_IN_CARAVAN)
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
    auto next_vertex = GetNextVertexInShortestPath(icebreaker.cur_pos, icebreaker, caravan, point);

    size_t icebreaker_graph_index = GetIcebreakerIndexByName(icebreaker.name);

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& graph = date_to_graph[okay_date].at(icebreaker_graph_index);

    Voyage voyage;
    voyage.start_time = cur_time;
    voyage.start_point = icebreaker.cur_pos;

    auto min_speed_ship = GetMinimalSpeedShipInCaravan(caravan);

    // GetEdgeWeight возвращает для ледоколов ВРЕМЯ, поэтому все ок, ни на что делить не нужно
    if (caravan.ships_id.empty() || min_speed_ship == nullptr) {
        // если караван пустой или ледокол самый медленный, скорость складывается только из веса ребер
        voyage.end_time = cur_time + GetEdgeWeight(graph, icebreaker.cur_pos, next_vertex.value());
    } else {
        // значит, самый медленный - корабль в караване
        size_t ship_index = ship_class_to_index.at(min_speed_ship->ice_class);
        auto& min_speed_ship_graph = date_to_graph.at(okay_date).at(ship_index);

        // все дебафы уже учтены в графе, просто нужно отдельно учитывать, что корабль в некоторых случаях не может идти по ребру сам
        voyage.end_time = cur_time + GetEdgeWeight(min_speed_ship_graph, icebreaker.cur_pos, next_vertex.value()) / min_speed_ship->speed;
    }

    voyage.end_point = next_vertex.value();

    icebreaker_to_voyage[icebreaker.id] = voyage;
    for (auto &ship_id: caravan.ships_id)
        ship_to_voyage[ship_id] = voyage;

    return voyage;
}

std::optional<VertID> PathManager::FindNewAchievablePoint(const Ship& ship, VertID from, std::unordered_set<VertID>& visited) {
    std::vector<VertID> neighbours;

    size_t ship_index = ship_class_to_index.at(ship.ice_class);

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& graph = date_to_graph[okay_date].at(ship_index);
    auto& distances = date_to_distances.at(okay_date).at(ship_index);

    visited.insert(from);

    for (auto edge : boost::make_iterator_range(boost::out_edges(from, graph))) {
        auto target = boost::target(edge, graph);
        if (visited.count(target) == 0) {
            neighbours.push_back(target);
        }
    }

    VertID optimal_new_finish = -1;
    bool found = false;

    for (auto neighbour : neighbours) {
        if (visited.count(neighbour) == 1) {
            continue;
        }

        visited.insert(neighbour);

        if (distances[ship.cur_pos][neighbour] < 1000.0f) {
            if (!found || distances[ship.cur_pos][neighbour] < distances[ship.cur_pos][optimal_new_finish]) {
                optimal_new_finish = neighbour;
                found = true;
            }
        }
    }

    if (found) {
        return optimal_new_finish;
    }

    for (auto neighbour : neighbours) {
        auto another_achievable = FindNewAchievablePoint(ship, neighbour, visited);
        if (another_achievable.has_value() && distances[ship.cur_pos][another_achievable.value()] < 1000.0f) {
            if (!found || distances[ship.cur_pos][another_achievable.value()] < distances[ship.cur_pos][optimal_new_finish]) {
                optimal_new_finish = another_achievable.value();
                found = true;
            }
        }
    }

    if (!found) {
        throw std::runtime_error("no new_finish thats nonsense");
    }

    return optimal_new_finish;
}

void PathManager::FixFinishForWeakShips(const std::vector<int>& weak_ships, const Icebreaker& icebreaker) {
    std::unordered_set<VertID> visited;
    for (auto weak_id : weak_ships) {
        auto time = TimeToArriveUnderFakeProvodka((*ships)[weak_id], icebreaker, (*ships)[weak_id].cur_pos, (*ships)[weak_id].finish);
        // finish is okay
        if (time < 10000.0f) {
            continue;
        }

        // now find
        auto new_finish = FindNewAchievablePoint((*ships)[weak_id], (*ships)[weak_id].finish, visited);
        if (!new_finish.has_value()) {
            throw std::runtime_error("new_finish == -1 it's nonsense");
        }
        (*ships)[weak_id].finish = new_finish.value();
    }
}

// build path to all icebreaker's caravan final points, return next step
Voyage PathManager::sail2depots(const Icebreaker &icebreaker, const Caravan &caravan) {
    std::vector<VertID> all_caravan_end_points;
    for (auto ship_id : caravan.ships_id) {
        all_caravan_end_points.push_back((*ships)[ship_id.id].finish);
    }

    std::vector<int> ship_ids_of_zero_ice_class;
    for (auto ship_id : caravan.ships_id) {
        if (int((*ships)[ship_id.id].ice_class) >= int(IceClass::kNoIceClass) && int((*ships)[ship_id.id].ice_class) <= int(IceClass::kArc3)) {
            ship_ids_of_zero_ice_class.push_back(ship_id.id);
        }
    }

    FixFinishForWeakShips(ship_ids_of_zero_ice_class, icebreaker);

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

std::optional<VertID> PathManager::GetNextVertexInShortestPath(VertID current,
        const Icebreaker& icebreaker, const Caravan& caravan, VertID end) const {
    std::optional<VertID> optimal_neighbour = std::nullopt;
    float optimal_metric = std::numeric_limits<float>::infinity();
    bool found = false;


    size_t icebreaker_graph_index = GetIcebreakerIndexByName(icebreaker.name);

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& icebreaker_graph = date_to_graph.at(okay_date).at(icebreaker_graph_index);

    auto min_speed_ship = GetMinimalSpeedShipInCaravan(caravan);

    for (auto neighbour : boost::make_iterator_range(boost::out_edges(current, icebreaker_graph))) {
        int target = boost::target(neighbour, icebreaker_graph);

        float metric;
        // GetEdgeWeight возвращает для ледоколов ВРЕМЯ, поэтому все ок, ни на что делить не нужно
        if (caravan.ships_id.empty() || min_speed_ship == nullptr) {
            // если караван пустой или ледокол самый медленный, скорость складывается только из весов ребер
            auto& icebreaker_distances = date_to_distances.at(okay_date).at(icebreaker_graph_index);
            metric = GetEdgeWeight(icebreaker_graph, current, target) + icebreaker_distances[target][end];
        } else {
            // значит самый медленный - один из кораблей, значит будем считать скорость каравана по нему
            auto& graph = date_to_graph.at(okay_date).at(ship_class_to_index.at(min_speed_ship->ice_class));
            auto& distances = date_to_distances.at(okay_date).at(ship_class_to_index.at(min_speed_ship->ice_class));

            // все дебафы уже учтены в графе, просто нужно отдельно учитывать, что корабль в некоторых случаях не может идти по ребру сам
            metric = GetEdgeWeight(graph, current, target) / min_speed_ship->speed + distances[target][end] / min_speed_ship->speed;
        }

        if (metric < optimal_metric) {
            optimal_neighbour = target;
            optimal_metric = metric;
            found = true;
        }
    }

    if (!found || optimal_metric >= 1000.0f) {
       return std::nullopt;
    }
    return optimal_neighbour;
}

std::optional<VertID> PathManager::GetNextVertexInShortestPathAlone(VertID current, const Ship& ship, VertID end) const {
    std::optional<VertID> optimal_neighbour = std::nullopt;
    float optimal_metric = 10000.0f;
    bool found = false;

    size_t alone_ship_graph_index = alone_ship_class_to_index.at(ship.ice_class);

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& graph = date_to_graph.at(okay_date).at(alone_ship_graph_index);
    auto& distances = date_to_distances.at(okay_date).at(alone_ship_graph_index);

    for (auto neighbour : boost::make_iterator_range(boost::out_edges(current, graph))) {
        VertID target = boost::target(neighbour, graph);
        float metric = GetEdgeWeight(graph, current, target) / ship.speed + distances[target][end] / ship.speed;

        if (metric < optimal_metric) {
            optimal_neighbour = target;
            optimal_metric = metric;
            found = true;
        }
    }

    if (!found || optimal_metric >= 1000.0f) {
        return std::nullopt;
    }

    return optimal_neighbour;
}

const Ship* PathManager::GetMinimalSpeedShipInCaravan(const Caravan& caravan) const {
    const Ship* min_speed_ship = nullptr;
    float min_speed = (*icebreakers)[caravan.icebreaker_id->id].speed;

    for (auto ship_id : caravan.ships_id) {
        if (const auto& ship = ships->at(ship_id.id); ship.speed < min_speed) {
            min_speed = ship.speed;
            min_speed_ship = &ship;
        }
    }

    return min_speed_ship;
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

std::pair<VertID, float> PathManager::GetNearestVertex(VertID source, const std::vector<VertID>& vertexes) const {
    if (vertexes.empty()) {
        throw std::runtime_error("GetNearestVertex(): unable to process empty vertexes vector");
    }

    auto okay_date = GetCurrentOkayDateByTime(cur_time);
    auto& distances = date_to_distances.at(okay_date)[GRAPH_CLASSES_AMOUNT - 1];

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

std::pair<float, std::vector<PDPPoint>> PathManager::TimeToSail(const Caravan& caravan) {
    if (!caravan.icebreaker_id.is_initialized()) {
        throw std::runtime_error("caravan's icebreaker is not initialized!");
    }
    if (caravan.ships_id.empty()) {
        return std::make_pair(0, std::vector<PDPPoint>{});
    }
    
    std::vector<PDPPoint> points;
    for (const auto& ship_id : caravan.ships_id) {
        const auto& ship = ships->at(ship_id.id);
        points.push_back(PDPPoint{ship.cur_pos, ship.id});
        points.push_back(PDPPoint{ship.finish, std::nullopt});
    }

    for (const auto& point : points) {
        if (point.vertex > 1000) {
            throw std::runtime_error("lol huy");
        }
    }

    const auto& icebreaker = icebreakers->at(caravan.icebreaker_id->id);
    
    float optimal_time = 10000.0f;
    std::vector<PDPPoint> optimal_points;

    const auto old_cur_time = cur_time;

    // изменяем все cur_pos в караване
    std::vector<std::pair<ShipId, VertID>> old_ships_cur_pos;
    std::pair<IcebreakerId, VertID> old_icebreaker_cur_pos = std::make_pair(icebreaker.id, icebreaker.cur_pos);
    {
        auto last_voyage = getCurrentVoyage(caravan);
        if (last_voyage.start_time != 0) {
            for (auto ship_id : caravan.ships_id) {
                old_ships_cur_pos.push_back(std::make_pair(ship_id, (*ships)[ship_id.id].cur_pos));
                (*ships)[ship_id.id].cur_pos = last_voyage.end_point;
            }
            (*icebreakers)[icebreaker.id.id].cur_pos = last_voyage.end_point;
            cur_time = last_voyage.end_time;
        }
    }

    size_t icebreaker_graph_index = GetIcebreakerIndexByName(icebreaker.name);
    VertID current_vertex = icebreaker.cur_pos;
    const Ship* min_speed_ship = nullptr;
    auto min_speed = icebreaker.speed;

    while (!points.empty()) {
        float optimal_time_to_next = 10000.0f;
        PDPPoint optimal_next_point;
        auto okay_date = GetCurrentOkayDateByTime(cur_time);

        if (min_speed_ship == nullptr) {
            for (size_t i = 0; i < points.size(); ++i) {
                const auto& icebreaker_distances = date_to_distances.at(okay_date).at(icebreaker_graph_index);
                auto time_to_next = icebreaker_distances[current_vertex][points[i].vertex];
                
                if (time_to_next < optimal_time_to_next) {
                    optimal_time_to_next = time_to_next;
                    optimal_next_point = points[i];
                }
            }
        }
        else {
            for (size_t i = 0; i < points.size(); ++i) {
                auto time_to_next = TimeToArriveUnderFakeProvodka(*min_speed_ship, icebreaker, current_vertex, points[i].vertex);
                
                if (time_to_next < optimal_time_to_next) {
                    optimal_time_to_next = time_to_next;
                    optimal_next_point = points[i];
                }
            }
        }

        optimal_time += optimal_time_to_next;
        optimal_points.push_back(optimal_next_point);

        if (optimal_next_point.ship_id.has_value()) {
            if (const auto& ship = ships->at(optimal_next_point.ship_id.value().id); ship.speed < min_speed) {
                min_speed = ship.speed;
                min_speed_ship = &ship;
            }
        }

        cur_time += optimal_time_to_next;

        points.erase(std::remove(points.begin(), points.end(), optimal_next_point), points.end());
    }

    // восстанавливаем положения судов в караване
    {
        (*icebreakers)[old_icebreaker_cur_pos.first.id].cur_pos = old_icebreaker_cur_pos.second;
        for (const auto& [ship_id, old_cur_pos] : old_ships_cur_pos) {
            (*ships)[ship_id.id].cur_pos = old_cur_pos;
        }
    }

    cur_time = old_cur_time;

    return std::make_pair(optimal_time, optimal_points);
}

// std::pair<float, std::vector<PDPPoint>> PathManager::TimeToSail(const Caravan& caravan) {
//     if (!caravan.icebreaker_id.is_initialized()) {
//         throw std::runtime_error("caravan's icebreaker is not initialized!");
//     }
//     if (caravan.ships_id.empty()) {
//         return std::make_pair(0, std::vector<PDPPoint>{});
//     }
    
//     std::vector<PDPPoint> points;
//     for (const auto& ship_id : caravan.ships_id) {
//         const auto& ship = ships->at(ship_id.id);
//         points.push_back(PDPPoint{ship.cur_pos, ship.id});
//         points.push_back(PDPPoint{ship.finish, std::nullopt});
//     }

//     for (const auto& point : points) {
//         if (point.vertex > 1000) {
//             throw std::runtime_error("lol huy");
//         }
//     }

//     std::sort(points.begin(), points.end());

//     const auto& icebreaker = icebreakers->at(caravan.icebreaker_id->id);
    
//     float optimal_time = 10000.0f;
//     std::vector<PDPPoint> optimal_points;

//     const auto old_cur_time = cur_time;

//     // изменяем все cur_pos в караване
//     std::vector<std::pair<ShipId, VertID>> old_ships_cur_pos;
//     std::pair<IcebreakerId, VertID> old_icebreaker_cur_pos = std::make_pair(icebreaker.id, icebreaker.cur_pos);
//     {
//         auto last_voyage = getCurrentVoyage(caravan);
//         if (last_voyage.start_time != 0) {
//             for (auto ship_id : caravan.ships_id) {
//                 old_ships_cur_pos.push_back(std::make_pair(ship_id, (*ships)[ship_id.id].cur_pos));
//                 (*ships)[ship_id.id].cur_pos = last_voyage.end_point;
//             }
//             (*icebreakers)[icebreaker.id.id].cur_pos = last_voyage.end_point;
//             cur_time = last_voyage.end_time;
//         }
//     }

//     auto process_points = [this](const Icebreaker& icebreaker, const std::vector<PDPPoint>& points) {
        // const Ship* min_speed_ship = nullptr;
        // auto min_speed = icebreaker.speed;
//         size_t icebreaker_graph_index = GetIcebreakerIndexByName(icebreaker.name);

//         float time = 0;
//         VertID current_vertex = icebreaker.cur_pos;

//         for (size_t i = 0; i < points.size(); ++i) {
//             auto okay_date = GetCurrentOkayDateByTime(cur_time);

//             float inc_time;
//             if (min_speed_ship == nullptr) {
                // const auto& icebreaker_distances = date_to_distances.at(okay_date).at(icebreaker_graph_index);
                // inc_time = icebreaker_distances[current_vertex][points[i].vertex];
//             } else {
//                 inc_time = TimeToArriveUnderFakeProvodka(*min_speed_ship, icebreaker, current_vertex, points[i].vertex);
//             }

            // if (points[i].ship_id.has_value()) {
            //     if (const auto& ship = ships->at(points[i].ship_id.value().id); ship.speed < min_speed) {
            //         min_speed = ship.speed;
            //         min_speed_ship = &ship;
            //     }
            // }

//             time += inc_time;
//             cur_time += inc_time;
//             current_vertex = points[i].vertex;
//         }

//         return std::make_pair(time, points);
//     };

//     do {
//         // TimerScope ts1("main points loop");
//         auto [time, points_sequence] = process_points(icebreaker, points);
//         if (time < optimal_time) {
//             optimal_time = time;
//             optimal_points = std::move(points_sequence);
//         }
//         cur_time = old_cur_time;
//     } while (std::next_permutation(points.begin(), points.end()));

//     std::cout << "\n\nloop done\n\n" << std::endl;

//     // восстанавливаем положения судов в караване
//     {
//         (*icebreakers)[old_icebreaker_cur_pos.first.id].cur_pos = old_icebreaker_cur_pos.second;
//         for (const auto& [ship_id, old_cur_pos] : old_ships_cur_pos) {
//             (*ships)[ship_id.id].cur_pos = old_cur_pos;
//         }
//     }

//     return std::make_pair(optimal_time, optimal_points);
// }

std::vector<Schedule> PathManager::SailPath(const Icebreaker& icebreaker__, const std::vector<PDPPoint>& points) {
    std::vector<Schedule> schedules;
    schedules.resize(1);

    const auto old_cur_time = cur_time;

    Icebreaker icebreaker = icebreaker__;

    Caravan current_caravan;
    current_caravan.icebreaker_id = icebreaker.id;

    // передвигаем ледокол
    auto old_icebreaker_cur_pos = std::make_pair(icebreaker.id, icebreaker.cur_pos);
    {
        auto last_voyage = getCurrentVoyage(current_caravan);
        if (last_voyage.start_time != 0) {
            icebreaker.cur_pos = last_voyage.end_point;
            (*icebreakers)[icebreaker.id.id].cur_pos = icebreaker.cur_pos;
            cur_time = last_voyage.end_time;
        }
    }

    VertID current_vert = icebreaker.cur_pos;

    for (size_t i = 0; i < points.size(); ++i) {
        // проверяем, могут ли корабли в караване доехать самостоятельно
        for (auto it = current_caravan.ships_id.begin(); it != current_caravan.ships_id.end(); ) {
            const auto& ship = ships->at(it->id);
            auto alone_path = GetShortestPathAlone(ship, current_vert, ship.finish);
            if (!alone_path.empty()) {
                schedules.push_back({});
                auto& schedule_alone = schedules.back();

                Caravan alone_caravan; alone_caravan.ships_id.insert(*it);
                for (auto&& voyage : std::move(alone_path)) {
                    schedule_alone.push_back({alone_caravan, std::move(voyage)});
                }

                it = current_caravan.ships_id.erase(it);
            } else {
                ++it;
            }
        }

        auto shortest_voyages = GetShortestPathForCaravan(current_caravan, current_vert, points[i].vertex);
        for (auto voyage : std::move(shortest_voyages)) {
            schedules[0].push_back({current_caravan, std::move(voyage)});
        }

        if (shortest_voyages.size() > 0) {
            cur_time = shortest_voyages.back().end_time;
        }

        if (points[i].ship_id.has_value()) {
            current_caravan.ships_id.insert(points[i].ship_id.value());
        }

        current_vert = points[i].vertex;
    }

    (*icebreakers)[old_icebreaker_cur_pos.first.id].cur_pos = old_icebreaker_cur_pos.second;

    cur_time = old_cur_time;
    return schedules;
}

std::vector<Voyage> PathManager::GetShortestPathForCaravan(const Caravan& caravan, VertID start, VertID end) {
    if (!caravan.icebreaker_id.is_initialized()) {
        throw std::runtime_error("GetShortestPathForCaravan(): caravan MUST have an icebreaker");
    }

    std::vector<Voyage> path;

    VertID current_vert = start;
    auto old_cur_time = cur_time;
    auto min_speed_ship = GetMinimalSpeedShipInCaravan(caravan);

    const auto& icebreaker = icebreakers->at(caravan.icebreaker_id.value().id);
    size_t icebreaker_index = GetIcebreakerIndexByName(icebreaker.name);

    while (current_vert != end) {
        auto next_vert = GetNextVertexInShortestPath(current_vert, icebreaker, caravan, end);
        if (!next_vert.has_value()) {
            return {};
        }

        Voyage current_voyage;
        current_voyage.start_point = current_vert;
        current_voyage.end_point = next_vert.value();
        current_voyage.start_time = cur_time;

        float time_to_next_vert;

        auto okay_date = GetCurrentOkayDateByTime(cur_time);
        if (min_speed_ship == nullptr) {
            const auto& graph = date_to_graph.at(okay_date).at(icebreaker_index);
            time_to_next_vert = GetEdgeWeight(graph, current_vert, next_vert.value());
        } else {
            const auto& graph = date_to_graph.at(okay_date).at(ship_class_to_index.at(min_speed_ship->ice_class));
            time_to_next_vert = GetEdgeWeight(graph, current_vert, next_vert.value()) / min_speed_ship->speed;
        }

        current_voyage.end_time = cur_time + time_to_next_vert;
        path.push_back(current_voyage);

        cur_time = current_voyage.end_time;
        current_vert = next_vert.value();
    }

    cur_time = old_cur_time;

    return path;
}

bool PathManager::HasEdge(const Icebreaker& icebreaker, Days time, VertID from, VertID to) const {
    size_t icebreaker_graph_index = GetIcebreakerIndexByName(icebreaker.name);
    auto okay_date = GetCurrentOkayDateByTime(time);
    const auto& graph = date_to_graph.at(okay_date).at(icebreaker_graph_index);
    return boost::edge(from, to, graph).second;
}
