#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

#include "src/parser.h"
#include "src/graph.h"

#include <boost/graph/graphviz.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif
    
    // auto ships = parser::ParseShipsSchedule("../dataset/Расписание движения судов.xlsx");
    // for (auto& ship : ships) {
    //     std::cout << ship.name << ": ice_class=" << static_cast<int>(ship.ice_class)
    //               << ", knot_speed=" << ship.knot_speed
    //               << ", departure=" << ship.departure
    //               << ", destination=" << ship.destination
    //               << ", voyage_start_date=" << ship.voyage_start_date
    //               << "\n" << std::endl;
    // }

    // auto icebreakers = parser::ParseIcebreakers("../dataset/Расписание движения судов.xlsx");
    // for (auto& icebreaker : icebreakers) {
    //     std::cout << icebreaker.name << ": "
    //               << "ice_class=" << static_cast<int>(icebreaker.ice_class)
    //               << ", knot_speed=" << icebreaker.knot_speed
    //               << ", departure=" << icebreaker.departure
    //               << "\n" << std::endl;
    // }

    auto graph = parser::ParseGraphFromExcel("../dataset/ГрафДанные.xlsx");
    std::ofstream graphviz_file{"graph_visual.dot"};
    boost::write_graphviz(graphviz_file, graph);
    graphviz_file.close();


    typedef boost::property_map<parser::Graph, boost::edge_weight_t>::type WeightMap;
    // Declare a matrix type and its corresponding property map that
    // will contain the distances between each pair of vertices.
    typedef boost::exterior_vertex_property<parser::Graph, double> DistanceProperty;
    typedef DistanceProperty::matrix_type DistanceMatrix;
    typedef DistanceProperty::matrix_map_type DistanceMatrixMap;

    WeightMap weight_pmap = boost::get(boost::edge_weight, graph);

    // set the distance matrix to receive the floyd warshall output
    DistanceMatrix distances(boost::num_vertices(graph));
    DistanceMatrixMap dm(distances, graph);

    // find all pairs shortest paths
    bool valid = floyd_warshall_all_pairs_shortest_paths(graph, dm, 
                                                boost::weight_map(weight_pmap));

    std::cout << "valid: " << valid << std::endl;

    int start = 43, end = 35;
    std::vector<int> vertexes_path;
    int current = start;

    while (current != end) {
        vertexes_path.push_back(current);

        std::cout << "current: " << current << std::endl;
        
        int optimal_neighbour = -1, optimal_metric = -1;
        for (auto neighbour : boost::make_iterator_range(boost::out_edges(current, graph))) {
            int target = boost::target(neighbour, graph);
            auto metric = GetEdgeWeight(graph, current, target) + distances[target][end];
            
            if (optimal_metric == -1 || metric < optimal_metric) {
                optimal_neighbour = target;
                optimal_metric = metric;
            }
        }

        if (optimal_neighbour == -1) {
            throw std::runtime_error("lol optimal neighbour is -1...");
        }

        current = optimal_neighbour;
    }
    vertexes_path.push_back(current);

    std::cout << "path: ";
    for (auto vertex : vertexes_path) {
        std::cout << vertex << " ";
    } std::cout << std::endl;

    return 0;
}