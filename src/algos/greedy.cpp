#include "algos.h"

#include <set>
#include <map>
#include <queue>
#include <list>
#include <unordered_map>
#include <boost/math/special_functions/factorials.hpp>
#include <iostream>

using WeightedBoats = std::map<BoatID, float, std::greater<float>>;

constexpr size_t MAX_CARAVAN_SIZE = 3;

float weightShipAlone(const Ship &ship, const Date &cur_time) {
    float w1 = 0.5; // mb 1.0 / max_speed
    float w2 = 0.5; // hz
    return w1 * ship.knot_speed + w2 * (cur_time - ship.voyage_start_date);
}

float weightShipForIcebreaker(const BoatInfo &ship, const BoatInfo &icebreaker) {
    return 0; // TODO
}

float weightIcebreaker(const BoatInfo &icebreaker, const WeightedBoats &ships_waiting, const Ships &ships) {
    return 0; // TODO
}

double ScoreCaravan() {

}

// это значение можно вычислить в compile-time.
double CountAllPossibleCaravans(size_t n, size_t k) {
    if (k == 0) return 0;
    return boost::math::factorial<double>(n) / (boost::math::factorial<double>(n - k) * boost::math::factorial<double>(k))
            + CountAllPossibleCaravans(n, k - 1);
}

struct ScoredCaravan {
    std::vector<BoatID> caravan;
    double score;
};

// https://rosettacode.org/wiki/Combinations#C.2B.2B
void CombinateAllPossibleCaravans(size_t N, size_t K, std::vector<ScoredCaravan>& all_possible_caravans)
{
    if (K == 0) {
        return;
    }

    std::vector<bool> v(N);
    std::fill(v.end() - K, v.end(), true);

    std::vector<BoatID> caravan; caravan.reserve(K); // оптимизация на std::array
    do {
        int caravan_ind = 0;
        for (int i = 0; i < N; ++i) {
            if (v[i]) {
                caravan[caravan_ind++] = i;
            }
        }
        
        all_possible_caravans.push_back(ScoredCaravan{caravan, 0});
    } while (std::next_permutation(v.begin(), v.end()));

    CombinateAllPossibleCaravans(N, K - 1, all_possible_caravans);
}

const int score_threshold = 50;

Schedule algos::greedy(const Ships &ships, const Icebreakers &icebreakers, PathManager &manager) {
    Schedule res;
    std::priority_queue<Date, std::vector<Date>, std::less<Date>> timestamps;
    timestamps.push(0); // TODO: get first timestamp from IntegrVelocity?

    // в начале все корабли БЕЗ караванов
    std::unordered_map<BoatID, bool> boat_is_alone;
    for (BoatID i = 0; i < ships.size(); ++i) {
        boat_is_alone[i] = true;
    }

    std::vector<ScoredCaravan> all_possible_caravans; // оптимальнее будет переписать на std::tuple или std::array
    size_t count_all_possible_caravans = CountAllPossibleCaravans(ships.size(), MAX_CARAVAN_SIZE);
    std::cout << "count_all_possible_caravans: " << count_all_possible_caravans << ", ships: " << ships.size() << std::endl;
    int _; std::cin >> _;

    all_possible_caravans.reserve(count_all_possible_caravans);

    // формируем все возможные караваны
    CombinateAllPossibleCaravans(ships.size(), MAX_CARAVAN_SIZE, all_possible_caravans);

    // оцениваем их (можно перенести оценку в CombinateAllPossibleCaravans, производительность может увеличиться)
    for (auto& scored_caravan : all_possible_caravans) {
        scored_caravan.score = ScoreCaravan(scored_caravan.caravan, ships, icebreakers, /* граф или другая мета-информация по нему */);
    }

    

    return {};

    // // TODO: actions before ships prepared

    // while (!ships.empty()) {
    //     // get next timestamp
    //     Date cur_time = timestamps.top();
    //     timestamps.pop();

    //     // get all waiting ships sorted by weight
    //     for (auto &ship: ships)
    //         if (!ships_waiting.count(ship.id) && ship.voyage_start_date <= cur_time)
    //             ships_waiting[ship.id] = weightShipAlone(ships[ship.id], cur_time);
        
    //     WeightedBoats icebreakers_waiting;
    //     // get all icebreakers in vertices sorted by weight
    //     for (auto &icebreaker: icebreakers) {
    //         auto last_voyage = manager.getCurrentVoyage(icebreaker.id);
    //         if (last_voyage.end_time == 0 || last_voyage.end_time <= cur_time) {
    //             // move caravan to vertex
    //             icebreaker.cur_pos = last_voyage.end_point;
    //             for (auto &ship_id: icebreaker.caravan)
    //                 ships[ship_id].cur_pos = last_voyage.end_point;
    //             icebreakers_waiting[icebreaker.id] = weightIcebreaker(icebreakers[icebreaker.id], ships_waiting, ships);
    //         }
    //     }

    //     // make decisions by priority        
    //     for (auto &icebreaker_id: icebreakers_waiting) {
    //         auto &icebreaker = icebreakers[icebreaker_id.first];
    //         WeightedBoats ships4icebreaker = ships_waiting;
    //         for (auto &ship: ships4icebreaker) {
    //             ship.second += weightShipForIcebreaker(ships[ship.first], icebreaker);
    //         }

    //         Ship &best_ship = ships[ships_waiting.begin()->first];
    //         float best_score = ships_waiting.begin()->second;
    //         Voyage decision;
            
    //         if
    //         (
    //             (best_score > score_threshold ||
    //             icebreaker.caravan.empty()) &&
    //             icebreaker.caravan.size() < MAX_SHIPS
    //         ) // move to the chosen ship
    //             decision = manager.sail2point(icebreaker, best_ship.cur_pos); 
    //         else // move to drop-off points
    //             decision = manager.sail2depots(icebreaker); 
            
    //         timestamps.push(decision.end_time);
    //         res.push_back({icebreaker.caravan, decision});
    //     }
    // }

    // return res;
}