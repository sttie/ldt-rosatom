#pragma once

#include "structs.h"
#include "path.h"

#include <vector>

namespace parser {

IcebreakersPtr ParseIcebreakers(const std::string& dataset_path, int &after_last_id);
ShipsPtr ParseShipsSchedule(const std::string& dataset_path, int start_id);

Graph ParseGraphFromExcel(const std::string& graph_filepath);

}
