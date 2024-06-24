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

template<typename T>
void combinateUtil(const std::vector<T> &arr, std::vector<T> data,
                    std::vector<std::set<T>> &res,
                    int start, int end,
                    int index, int size) 
{ 
    if (index == size) 
    {
        res.emplace_back(data.begin(), data.end());
        return;
    }

    for (size_t i = start; i <= end && 
        end - i + 1 >= size - index; i++) 
    { 
        data[index] = arr[i]; 
        combinateUtil(arr, data, res, i+1, 
                        end, index+1, size); 
    }
}

template<typename T>
std::vector<std::set<T>> getAllCombinations(const std::vector<T> &arr, size_t max_size) {
    std::vector<T> data(max_size, 0);
    std::vector<std::set<T>> result;

    combinateUtil<T>(arr, data, result, 0, arr.size() - 1, 0, max_size);

    return result;
}

std::vector<std::pair<size_t, size_t>> AssignmentCP(const std::vector<std::vector<float>> &costs,
                                            const std::vector<std::pair<size_t, size_t>> &banned_pairs);

namespace algos {

Schedule greedy(PathManager &manager, double * sum_res);
Schedule pseudo_exhaustive(PathManager &manager, double *sum_res);

} //algos