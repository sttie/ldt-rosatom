#pragma once

#include <vector>
#include <iostream>

template <typename Weight>
class Graph {
    std::vector<std::vector<Weight>> adjacency_matrix;

public:
    Graph(size_t vertexes_num) {
        adjacency_matrix.resize(vertexes_num, std::vector<Weight>(vertexes_num, Weight{}));
    }

    void AddEdge(size_t from, size_t to, Weight weight) {
        if (from >= adjacency_matrix.size() || to >= adjacency_matrix.size()) {
            throw std::runtime_error(
                "from: " + std::to_string(from) + 
                ", to: " + std::to_string(to)   + 
                ", size: " + std::to_string(adjacency_matrix.size()));
        }

        adjacency_matrix[from][to] = weight;
        // неориентированный!
        adjacency_matrix[to][from] = weight;
    }

    Weight GetEdge(size_t from, size_t to) const {
        if (from >= adjacency_matrix.size() || to >= adjacency_matrix.size()) {
            throw std::runtime_error(
                "from: " + std::to_string(from) + 
                ", to: " + std::to_string(to)   + 
                ", size: " + std::to_string(adjacency_matrix.size()));
        }

        return adjacency_matrix[from][to];
    }

    void Print() {
        for (size_t i = 0; i < adjacency_matrix.size(); ++i) {
            std::cout << i << ": ";
            for (size_t j = 0; j < adjacency_matrix.size(); ++j) {
                if (i == j) continue;

                if (adjacency_matrix[i][j] > 0) {
                    std::cout << j << " ";
                }
            }
            std::cout << std::endl;
        }
    }
};
