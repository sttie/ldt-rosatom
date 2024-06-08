#pragma once

#include <vector>

namespace matrix_utils {

// template <typename T>
// struct MatrixTrait {
//     using Matrix = std::vector<std::vector<T>>;
//     using Cell = std::pair<T, T>;
// };

template <typename T>
using Matrix = std::vector<std::vector<T>>;
using Cell = std::pair<size_t, size_t>;

}
