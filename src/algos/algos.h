#include "structs.h"

#include <map>

#include "path.h"

template<typename K, typename V>
std::vector<std::pair<K, V>> sortMapLess(const std::map<K, V> &m) {
    auto cmp = [](std::pair<K,V> const &a, std::pair<K,V> const &b) 
    { 
        return a.second == b.second ? a.first < b.first : a.second < b.second;
    };
    std::vector<std::pair<K, V>> vec_sorted;
    for (const auto &val: m)
        vec_sorted.push_back(val);
    std::sort(vec_sorted.begin(), vec_sorted.end(), cmp);
    return vec_sorted;
}

namespace algos {

Schedule greedy(PathManager &manager, double * sum_res);

} //algos