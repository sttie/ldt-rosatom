#pragma once

#include "structs.h"
#include "graph/graph.h"

#include <vector>

namespace parser {

using Ships = std::vector<Ship>;
using Icebreakers = std::vector<Icebreaker>;

Ships ParseShipsSchedule(const std::string& dataset_path);
Icebreakers ParseIcebreakers(const std::string& dataset_path);

Graph<double> ParseGraphFromExcel(const std::string& graph_filepath);

}
