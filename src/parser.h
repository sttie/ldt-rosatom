#pragma once

#include "structs.h"
#include "path.h"

#include <vector>

namespace parser {

GraphPointsInfo ParseGraphPointsFromExcel(const std::string& graph_filepath);
DatesToIceGraph ParseGraphFromJson(
        const std::string& vertices_filepath, const std::string& edges_filepath, IcebreakersPtr icebreakers);

IceGrid ParseIceGrid(const std::string& ice_filepath);

IcebreakersPtr ParseIcebreakers(const std::string& dataset_path, const GraphPointsInfo& graph_points_info);
ShipsPtr ParseShipsSchedule(const std::string& dataset_path, const GraphPointsInfo& graph_points_info);

}
