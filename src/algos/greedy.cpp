#include "algos.h"

#include <set>
#include <map>
#include <queue>
#include <iostream>

using WeightedBoats = std::map<BoatID, float, std::greater<float>>;

float weightShipAlone(const Ship &ship, const Date &cur_time, float max_speed, double cur_max_waiting_time) {
    float
        w1 = 0.2, speed_coef = ship.knot_speed / max_speed,
        w2 = 0.3, waiting_coef = difftime(cur_time, ship.voyage_start_date) / cur_max_waiting_time;
    return w1 * speed_coef + w2 * waiting_coef;
}

float weightShipForIcebreaker(const Icebreaker &icebreaker, const Ship &ship, const PathManager &pm) {
    // TODO: mb
    float w3 = 0.5;

    // check how longer distance to all drop-off points will become after picking-up this ship
    std::vector<VertID> cur_route;
    for (auto &ship_id: icebreaker.caravan)
        cur_route.push_back((pm.ships.get())->at(ship_id).finish);
    double dist1 = pm.PathDistance(icebreaker.cur_pos, cur_route);
    cur_route.push_back(ship.finish);
    double dist2 = pm.PathDistance(icebreaker.cur_pos, cur_route);

    float longination_coef = dist1 / dist2; // smaller/bigger
    return w3 * longination_coef;
}

float weightIcebreaker(const Icebreaker &icebreaker, const PathManager &pm, const std::set<VertID> &in_caravans) {
    float caravan_coef = (MAX_SHIPS - icebreaker.caravan.size()) / MAX_SHIPS;

    std::vector<VertID> waiting_positions;
    std::vector<BoatID> waiting_ships;
    for (const auto &ship: *pm.ships) {
        if (!in_caravans.count(ship.id)) {
            waiting_ships.push_back(ship.id);
            waiting_positions.push_back(ship.cur_pos);
        }
    }
    auto [vertex, distance] = pm.GetNearestVertex(icebreaker.cur_pos, waiting_positions);

    float dist_coef = distance * (0.5 + 0.5 * caravan_coef);

    return dist_coef; // TODO
}

const int score_threshold = 0.5;

Schedule algos::greedy(PathManager &manager) {
    Ships &ships = *manager.ships;
    Icebreakers &icebreakers = *manager.icebreakers;

    Schedule res;
    std::priority_queue<Date, std::vector<Date>, std::less<Date>> timestamps;
    for (const auto& ship : ships) {
        // timestamps.push(0); // TODO: get first timestamp from IntegrVelocity?
        std::cout << "voyage_start_date: " << ship.voyage_start_date << std::endl;
        timestamps.push(ship.voyage_start_date);
    }

    WeightedBoats ships_waiting; // not arrived to depot: waiting + in travel
    std::set<BoatID> ships_in_caravans;

    float max_speed = 0;
    for (auto &ship: ships)
        if (ship.knot_speed > max_speed)
            max_speed = ship.knot_speed;

    // TODO: actions before ships prepared
    // TODO: actions for arrived ship

    while (!ships_waiting.empty()) {
        // get next timestamp
        Date cur_time = timestamps.top();
        timestamps.pop();
        
        WeightedBoats icebreakers_waiting;
        // get all arrived in vertices icebreakers sorted by weight
        for (auto &icebreaker: icebreakers) {
            auto last_voyage = manager.getCurrentVoyage(icebreaker.id);
            if (last_voyage.end_time == 0 || last_voyage.end_time <= cur_time) {
                
                // move caravan to vertex
                icebreaker.cur_pos = last_voyage.end_point;
                for (auto &ship_id: icebreaker.caravan) {
                    ships[ship_id].cur_pos = last_voyage.end_point;
                    
                    // remove arrived ships
                    if (ships[ship_id].finish == ships[ship_id].cur_pos) {
                        ships_waiting.erase(ship_id);
                        icebreaker.caravan.erase(ship_id);
                    }

                    // pickup
                    if (icebreaker.to_pickup == ship_id && icebreaker.cur_pos == ships[ship_id].cur_pos) {
                        icebreaker.to_pickup = 0;
                        icebreaker.caravan.insert(ship_id);
                        ships_in_caravans.insert(ship_id);
                    }
                }

                // need to process this icebreaker
                icebreakers_waiting[icebreaker.id] = weightIcebreaker(icebreakers[icebreaker.id], manager, ships_in_caravans);
            }
        }

        // calc max waiting time
        double cur_max_wait = 0;
        for (auto &ship_id: ships_waiting) {
            double diff = difftime(cur_time, ships[ship_id.first].voyage_start_date);
            if (diff > cur_max_wait)
                cur_max_wait = diff;
        }

        // get all waiting ships sorted by weight
        for (auto &ship: ships)
            if (!ships_waiting.count(ship.id) && ship.voyage_start_date <= cur_time)
                if (manager.getCurrentVoyage(ship.id).end_time == 0)
                    ships_waiting[ship.id] = weightShipAlone(ships[ship.id], cur_time, max_speed, cur_max_wait);

        // make decisions by priority        
        for (auto &icebreaker_id: icebreakers_waiting) {
            auto &icebreaker = icebreakers[icebreaker_id.first];
            WeightedBoats ships4icebreaker = ships_waiting;
            for (auto &ship: ships4icebreaker) {
                ship.second += weightShipForIcebreaker(icebreaker, ships[ship.first], manager);
            }

            Voyage decision;
            bool choose_pickup = false;

            // check all ships by priority
            for (auto &best_ship: ships_waiting) {
                Ship &cur_ship = ships[best_ship.first];
                float best_score = best_ship.second;

                if (ships_in_caravans.count(cur_ship.id))
                    continue;
            
                if ((best_score > score_threshold || icebreaker.caravan.empty()) &&
                    icebreaker.caravan.size() < MAX_SHIPS)
                { // move to the chosen ship
                    decision = manager.sail2point(icebreaker, cur_ship.cur_pos, cur_time); 
                    icebreaker.to_pickup = cur_ship.id;
                    choose_pickup = true;
                    break;
                }
            }
            if (!choose_pickup) // move to drop-off points
                decision = manager.sail2depots(icebreaker, cur_time); 
            
            timestamps.push(decision.end_time);
            res.push_back({icebreaker.caravan, decision});
        }
    }

    return res;
}