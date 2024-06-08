#pragma once

#include "matrix.h"

#include <vector>

using IceMap = matrix_utils::Matrix<double>;

void SplitIceMapToPolygons(IceMap ice_map, const std::vector<std::pair<double, double>>& classes);
