#pragma once

#include "structs.h"

#include <vector>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

namespace parser {

using EdgeWeightProperty = boost::property<boost::edge_weight_t, double>;
using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, EdgeWeightProperty>;

using Ships = std::vector<Ship>;
using Icebreakers = std::vector<Icebreaker>;

Ships ParseShipsSchedule(const std::string& dataset_path);
Icebreakers ParseIcebreakers(const std::string& dataset_path);

Graph ParseGraphFromExcel(const std::string& graph_filepath);

}
