#pragma once

#include "structs.h"
#include "graph/graph.h"

#include <vector>

namespace parser {

Icebreakers ParseIcebreakers(const std::string& dataset_path, int &after_last_id);
Ships ParseShipsSchedule(const std::string& dataset_path, int start_id);

Graph<float> ParseGraphFromExcel(const std::string& graph_filepath);

}
