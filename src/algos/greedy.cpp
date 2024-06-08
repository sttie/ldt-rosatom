#include "algos.h"

#include <set>
#include <map>
#include <queue>

using WeightedBoats = std::map<BoatID, float, std::greater<float>>;

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

const int score_threshold = 50;

Schedule algos::greedy(Ships &ships, Icebreakers &icebreakers, PathManager &manager) {
    Schedule res;
    std::priority_queue<Date, std::vector<Date>, std::less<Date>> timestamps;
    timestamps.push(0); // TODO: get first timestamp from IntegrVelocity?

    WeightedBoats ships_waiting;

    // TODO: actions before ships prepared

    while (!ships.empty()) {
        // get next timestamp
        Date cur_time = timestamps.top();
        timestamps.pop();

        // get all waiting ships sorted by weight
        for (auto &ship: ships)
            if (!ships_waiting.count(ship.id) && ship.voyage_start_date <= cur_time)
                ships_waiting[ship.id] = weightShipAlone(ships[ship.id], cur_time);
        
        WeightedBoats icebreakers_waiting;
        // get all icebreakers in vertices sorted by weight
        for (auto &icebreaker: icebreakers) {
            auto last_voyage = manager.getCurrentVoyage(icebreaker.id);
            if (last_voyage.end_time == 0 || last_voyage.end_time <= cur_time) {
                // move caravan to vertex
                icebreaker.cur_pos = last_voyage.end_point;
                for (auto &ship_id: icebreaker.caravan)
                    ships[ship_id].cur_pos = last_voyage.end_point;
                icebreakers_waiting[icebreaker.id] = weightIcebreaker(icebreakers[icebreaker.id], ships_waiting, ships);
            }
        }

        // make decisions by priority        
        for (auto &icebreaker_id: icebreakers_waiting) {
            auto &icebreaker = icebreakers[icebreaker_id.second];
            WeightedBoats ships4icebreaker = ships_waiting;
            for (auto &ship: ships4icebreaker) {
                ship.second += weightShipForIcebreaker(ships[ship.first], icebreaker);
            }

            Ship &best_ship = ships[ships_waiting.begin()->first];
            float best_score = ships_waiting.begin()->second;
            Voyage decision;
            
            if
            (
                (best_score > score_threshold ||
                icebreaker.caravan.empty()) &&
                icebreaker.caravan.size() < MAX_SHIPS
            ) // move to the chosen ship
                decision = manager.sail2point(icebreaker, best_ship.cur_pos); 
            else // move to drop-off points
                decision = manager.sail2depots(icebreaker); 
            
            timestamps.push(decision.end_time);
            res.push_back({icebreaker.caravan, decision});
        }
    }

    return res;
}