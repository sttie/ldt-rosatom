#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

#include "src/parser.h"
#include "algos/algos.h"

#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <json.hpp>
#include <fstream>
#include <string>
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

int main(int argc, char* argv[]) {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif

    std::string graphDataPath, scheduleShipsPath;

    if(argc > 1) {
        graphDataPath = argv[1];
        scheduleShipsPath = argv[2];
        if(graphDataPath[0] == '"') {
            graphDataPath = graphDataPath.substr(1, graphDataPath.size() - 2);
        }
        if(scheduleShipsPath[0] == '"'){
            scheduleShipsPath = scheduleShipsPath.substr(1, scheduleShipsPath.size() - 2);
        }
        std::cout << graphDataPath << " " << scheduleShipsPath << std::endl;
    } else {
        graphDataPath = "../dataset/ГрафДанные.xlsx";
        scheduleShipsPath = "../dataset/Расписание движения судов.xlsx";
    }
    
    /** Parsing input data **/

    GraphPointsInfo graph_points = parser::ParseGraphPointsFromExcel(graphDataPath);
    // auto ice_grid = parser::ParseIceGrid("../dataset/IntegrVelocity.xlsx");
    std::cout << "ice grid has been read!" << std::endl;

    // auto icebreakers = parser::ParseIcebreakers("../dataset/ScheduleTest.xlsx", graph_points);
    auto icebreakers = parser::ParseIcebreakers(scheduleShipsPath, graph_points);
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
    auto ships = parser::ParseShipsSchedule(scheduleShipsPath, graph_points);
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

    auto graph = parser::ParseGraphFromJson("../run/vertices.json", "../run/edges.json", icebreakers);

    /** Дополнительная подготовка данных **/


    json res_json;
    res_json["icebreakers"] = json::array();
    for (size_t i = 0; i < (*icebreakers).size(); i++) {
        res_json["icebreakers"].push_back({{"id",i}, {"name", (*icebreakers)[i].name}, {"path", json::array()}});
    }
    for (size_t i = 0; i < (*ships).size(); i++) {
        res_json["ships"].push_back({{"id", i}, {"name", (*ships)[i].name}, {"path", json::array()}});
        res_json["ships"][i]["path"].push_back({
                                                        {"caravan",""},
                                                        {"start", (*ships)[i].cur_pos},
                                                        {"start_time",(*ships)[i].voyage_start_date},
                                                        {"end", (*ships)[i].cur_pos},
                                                        {"end_time",(*ships)[i].voyage_start_date}
                                                        });

    }

    // /** Алгоритм **/
    PathManager pm(std::move(graph), icebreakers, ships);

    double sum;

    Schedule res = algos::greedy(pm, &sum);


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
        } else {
            res_json["ships"][sch_atom.caravan.ships_id.begin()->id]["path"].push_back({
                                                                    {"caravan", ""},
                                                                    {"start", start},
                                                                    {"end", end},
                                                                    {"start_time", sch_atom.edge_voyage.start_time},
                                                                    {"end_time", sch_atom.edge_voyage.end_time}
            });
        }
        schedule << CaravanToString(sch_atom.caravan.ships_id) << ": " << start << " -> " << end;
        schedule << " (" << sch_atom.edge_voyage.start_time << ";" << sch_atom.edge_voyage.end_time << ")";
        schedule << "\n";
    }

    std::ifstream ifs_vertices("../run/vertices.json");
    json jf_vertices = json::parse(ifs_vertices);

    std::ifstream ifs_edges("../run/edges.json");
    json jf_edges = json::parse(ifs_edges);

    res_json["edges"] = jf_edges;
    res_json["vertices"] = jf_vertices;
    res_json["sum"] = sum;

    std::ofstream o("../run/schedule_raw.json");
    o << std::setw(4) << res_json << std::endl;
    schedule.close();

    return 0;
}