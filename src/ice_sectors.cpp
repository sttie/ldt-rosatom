#include "ice_sectors.h"

#include <queue>
#include <list>

// 1. Условие, при котором клетка кладется в массив 

namespace {

constexpr size_t OPTIMAL_FREE_CELLS_SIZE = 100;

}

void SplitIceMapToPolygons(IceMap ice_map, const std::vector<std::pair<double, double>>& classes) {
    std::queue<matrix_utils::Cell, std::list<matrix_utils::Cell>> free_cells;
    bfs();
}
