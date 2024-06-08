#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

#include "src/parser.h"
#include "algos/algos.h"

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif
    
 int index = 0;
    
    /** Parsing input data **/

    auto icebreakers = parser::ParseIcebreakers("/mnt/c/Work/Projects/ldt-rosatom/dataset/Расписание движения судов.xlsx", index);
    for (auto& icebreaker : icebreakers) {
        std::cout << icebreaker.id << " "
                  << icebreaker.name << ": "
                  << "ice_class=" << static_cast<int>(icebreaker.ice_class)
                  << ", knot_speed=" << icebreaker.knot_speed
                  << ", departure=" << icebreaker.cur_pos
                  << "\n" << std::endl;
    }

    auto ships = parser::ParseShipsSchedule("/mnt/c/Work/Projects/ldt-rosatom/dataset/Расписание движения судов.xlsx", index);
    for (auto& ship : ships) {
        std::cout << ship.id << " "
                  << ship.name << ": ice_class=" << static_cast<int>(ship.ice_class)
                  << ", knot_speed=" << ship.knot_speed
                  << ", departure=" << ship.cur_pos
                  << ", destination=" << ship.finish
                  << ", voyage_start_date=" << ship.voyage_start_date
                  << "\n" << std::endl;
    }

    PathGraph graph = parser::ParseGraphFromExcel("../dataset/ГрафДанные.xlsx");
    graph.Print();

    // парсить сетку
    IceGrid ice_grid; // = ... TODO

    /** Дополнительная подготовка данных **/

    /** Алгоритм **/
    PathManager pm(graph, ice_grid);
    // Schedule res = algos::greedy(ships, icebreakers, pm);

    return 0;
}