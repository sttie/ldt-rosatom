#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

#include "src/parser.h"
#include "algos/algos.h"

#include <boost/graph/graphviz.hpp>

void GenerateGraph(Graph& graph) {
    boost::add_edge(5, 1, 116.74, graph);
    boost::add_edge(0, 2, 260.18, graph);
    boost::add_edge(0, 5, 157.6, graph);
    boost::add_edge(0, 4, 192.45, graph);
    boost::add_edge(2, 3, 409.31, graph);
    boost::add_edge(4, 1, 200.31, graph);
    boost::add_edge(0, 1, 600.31, graph);
    boost::add_edge(5, 6, 191.31, graph);
    boost::add_edge(2, 6, 50.31, graph);
}

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif
    
 int index = 0;
    
    /** Parsing input data **/

    auto icebreakers = parser::ParseIcebreakers("../dataset/ScheduleTest.xlsx", index);
    for (auto& icebreaker : *icebreakers) {
        std::cout << icebreaker.id << " "
                  << icebreaker.name << ": "
                  << "ice_class=" << static_cast<int>(icebreaker.ice_class)
                  << ", knot_speed=" << icebreaker.knot_speed
                  << ", departure=" << icebreaker.cur_pos
                  << "\n" << std::endl;
    }

    auto ships = parser::ParseShipsSchedule("../dataset/ScheduleTest.xlsx", index);
    for (auto& ship : *ships) {
        std::cout << ship.id << " "
                  << ship.name << ": ice_class=" << static_cast<int>(ship.ice_class)
                  << ", knot_speed=" << ship.knot_speed
                  << ", departure=" << ship.cur_pos
                  << ", destination=" << ship.finish
                  << ", voyage_start_date=" << ship.voyage_start_date
                  << "\n" << std::endl;
    }

    // ТЕСТ!!!
    (*ships)[0].cur_pos = 0; (*ships)[0].finish = 1;
    (*ships)[1].cur_pos = 4; (*ships)[1].finish = 3;
    (*ships)[2].cur_pos = 2; (*ships)[2].finish = 1;

    (*icebreakers)[0].cur_pos = 4;
    (*icebreakers)[1].cur_pos = 2;

    // Graph graph = parser::ParseGraphFromExcel("../dataset/ГрафДанные.xlsx");
    Graph graph;
    GenerateGraph(graph);
    std::ofstream graphviz_file{"graph_visual.dot"};
    boost::write_graphviz(graphviz_file, graph);
    graphviz_file.close();

    // парсить сетку
    // IceGrid ice_grid; // = ... TODO

    /** Дополнительная подготовка данных **/

    /** Алгоритм **/
    PathManager pm(graph, icebreakers, ships);
    Schedule res = algos::greedy(pm);

    std::cout << "schedule size: " << res.size() << std::endl;
    for (const auto& [caravan, voyage] : res) {
        auto start = voyage.start_point, end = voyage.end_point;
        std::cout << CaravanToString(caravan) << ": " << start << " -> " << end << std::endl;
    }

    return 0;
}