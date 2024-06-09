#pragma once

#include "structs.h"
#include "path.h"

#include <vector>

namespace parser {

IcebreakersPtr ParseIcebreakers(const std::string& dataset_path);
ShipsPtr ParseShipsSchedule(const std::string& dataset_path);

Graph ParseGraphFromExcel(const std::string& graph_filepath);

}
