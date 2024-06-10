#pragma once

#include "structs.h"
#include "path.h"

#include <vector>

namespace parser {

GraphPointsInfo ParseGraphPointsFromExcel(const std::string& graph_filepath);
Graph ParseGraphFromExcel(const std::string& graph_filepath);

IceGrid ParseIceGrid(const std::string& ice_filepath);

IcebreakersPtr ParseIcebreakers(const std::string& dataset_path, const GraphPointsInfo& graph_points_info);
ShipsPtr ParseShipsSchedule(const std::string& dataset_path, const GraphPointsInfo& graph_points_info);

}
