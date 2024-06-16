#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

#include "src/parser.h"
#include "algos/algos.h"

#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <json.hpp>
using json = nlohmann::json;

// void GenerateGraph(Graph& graph) {
    // boost::add_edge(5, 1, 116.74, graph);
    // boost::add_edge(0, 2, 260.18, graph);
    // boost::add_edge(0, 5, 157.6, graph);
    // boost::add_edge(0, 4, 192.45, graph);
    // boost::add_edge(2, 3, 409.31, graph);
    // boost::add_edge(4, 1, 200.31, graph);
    // boost::add_edge(0, 1, 600.31, graph);
    // boost::add_edge(5, 6, 191.31, graph);
    // boost::add_edge(2, 6, 50.31, graph);
// }

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif
    
    /** Parsing input data **/

    GraphPointsInfo graph_points = parser::ParseGraphPointsFromExcel("../dataset/ГрафДанные.xlsx");
    // auto ice_grid = parser::ParseIceGrid("../dataset/IntegrVelocity.xlsx");
    std::cout << "ice grid has been read!" << std::endl;

    // auto icebreakers = parser::ParseIcebreakers("../dataset/ScheduleTest.xlsx", graph_points);
    auto icebreakers = parser::ParseIcebreakers("../dataset/Расписание движения судов.xlsx", graph_points);
    // for (auto& icebreaker : *icebreakers) {
    //     std::cout << icebreaker.id.id << " "
    //               << icebreaker.name << ": "
    //               << "ice_class=" << static_cast<int>(icebreaker.ice_class)
    //               << ", knot_speed=" << icebreaker.knot_speed
    //               << ", departure=" << icebreaker.cur_pos
    //               << "\n" << std::endl;
    // }
     
    std::cout << "icebreakers has been read!" << std::endl;

    // auto ships = parser::ParseShipsSchedule("../dataset/ScheduleTest.xlsx", graph_points);
    auto ships = parser::ParseShipsSchedule("../dataset/Расписание движения судов.xlsx", graph_points);
    // for (auto& ship : *ships) {
    //     std::cout << ship.id.id << " "
    //               << ship.name << ": ice_class=" << static_cast<int>(ship.ice_class)
    //               << ", knot_speed=" << ship.knot_speed
    //               << ", departure=" << ship.cur_pos
    //               << ", destination=" << ship.finish
    //               << ", voyage_start_date=" << ship.voyage_start_date
    //               << "\n" << std::endl;
    // }

    std::cout << "ships has been read!" << std::endl;

    auto graph = parser::ParseGraphFromJson("../dataset/vertices.json", "../dataset/edges.json", icebreakers);

    /** Дополнительная подготовка данных **/

    // /** Алгоритм **/
    PathManager pm(std::move(graph), icebreakers, ships);

    Schedule res = algos::greedy(pm);
    json res_json;
    res_json["icebreakers"] = json::array();
    for (int i = 0; i < (*icebreakers).size(); i++) {
        res_json["icebreakers"].push_back({{"id",i}, {"path", json::array()}});
    }

    std::ofstream schedule("schedule.txt");
    schedule << "schedule size: " << res.size() << std::endl;
    for (const auto& sch_atom: res) {
        auto start = sch_atom.edge_voyage.start_point, end = sch_atom.edge_voyage.end_point;
        if (sch_atom.caravan.icebreaker_id.is_initialized()){
                std::vector<int> carvs;
                for (auto it = sch_atom.caravan.ships_id.begin(); it != sch_atom.caravan.ships_id.end(); ++it) {
                    carvs.push_back(it->id);
                }
                res_json["icebreakers"][sch_atom.caravan.icebreaker_id->id]["path"].push_back({
                                                   {"caravan", carvs}, 
                                                   {"start", start},
                                                   {"end", end},
                                                   {"start_time",sch_atom.edge_voyage.start_time},
                                                   {"end_time", sch_atom.edge_voyage.end_time}});
                schedule << "[" << std::to_string(sch_atom.caravan.icebreaker_id->id) << "] ";
        }
        schedule << CaravanToString(sch_atom.caravan.ships_id) << ": " << start << " -> " << end;
        schedule << " (" << sch_atom.edge_voyage.start_time << ";" << sch_atom.edge_voyage.end_time << ")";
        schedule << "\n";
    }
    std::ofstream o("schedule.json");
    o << std::setw(4) << res_json << std::endl;
    schedule.close();

    std::cout << "done!" << std::endl;
    int _; std::cin >> _;

    return 0;
}