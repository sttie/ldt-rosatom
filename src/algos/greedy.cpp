#include "algos.h"

#include <set>
#include <map>
#include <queue>
#include <iostream>

using WeightedShips = std::map<ShipId, float>; //todo
using WeightedIcebreakers = std::map<IcebreakerId, float>; //todo

float weightShipAlone(const Ship &ship, const Date &cur_time, float max_speed, double cur_max_waiting_time) {
    float
        w1 = 0.2, speed_coef = ship.knot_speed / max_speed,
        w2 = 0.3, waiting_coef = cur_max_waiting_time != 0 ? difftime(cur_time, ship.voyage_start_date) / cur_max_waiting_time : 0;
    return w1 * speed_coef + w2 * waiting_coef;
}

float weightShipForIcebreaker(const Icebreaker &icebreaker, const Ship &ship, const PathManager &pm) {
    // TODO: mb add distance icebreaker-ship

    float w3 = 0.5;

    // check how longer distance to all drop-off points will become after picking-up this ship
    std::vector<VertID> cur_route;
    for (const auto& ship_id : icebreaker.caravan.ships_id)
        cur_route.push_back(pm.ships.get()->at(ship_id.id).finish);

    float longination_coef = 1;
    if (!cur_route.empty()) {
        double dist1 = pm.PathDistance(icebreaker.cur_pos, cur_route);
        cur_route.push_back(ship.finish);
        double dist2 = pm.PathDistance(icebreaker.cur_pos, cur_route);

        longination_coef = dist1 / dist2; // smaller/bigger
    }

    return w3 * longination_coef;
}

float weightIcebreaker(const Icebreaker &icebreaker, const PathManager &pm, const std::set<ShipId> &in_caravans) {
    float caravan_coef = (MAX_SHIPS - icebreaker.caravan.ships_id.size()) / MAX_SHIPS;

    std::vector<VertID> waiting_positions;
    std::vector<ShipId> waiting_ships;
    for (const auto &ship: *pm.ships) {
        if (!in_caravans.count(ship.id)) {
            waiting_ships.push_back(ship.id);
            waiting_positions.push_back(ship.cur_pos);
        }
    }
    if (!waiting_positions.empty()) {
        auto [vertex, distance] = pm.GetNearestVertex(icebreaker.cur_pos, waiting_positions);
        float dist_coef = caravan_coef; // TODO distance * (0.5 + 0.5 * caravan_coef);
        return dist_coef; // TODO
    }
    else
        return 0;
}

const double score_threshold = 0.1;

Schedule algos::greedy(PathManager &manager) {
    Ships &ships = *manager.ships;
    Icebreakers &icebreakers = *manager.icebreakers;

    Schedule res;
    std::priority_queue<Date, std::vector<Date>, std::greater<Date>> timestamps;
    for (const auto& ship : ships) {
        // timestamps.push(0); // TODO: get first timestamp from IntegrVelocity?
        std::cout << "voyage_start_date: " << ship.voyage_start_date << std::endl;
        timestamps.push(ship.voyage_start_date);
    }

    WeightedShips ships_waiting; // not arrived to depot: waiting + in travel
    std::set<ShipId> ships_in_caravans;

    float max_speed = 0;
    for (auto &ship: ships)
        if (ship.knot_speed > max_speed)
            max_speed = ship.knot_speed;

    // TODO: actions before ships prepared
    // TODO: actions for arrived ship

    while (!timestamps.empty()) {
        // get next timestamp
        Date cur_time = timestamps.top();
        while (!timestamps.empty() && cur_time == timestamps.top())
            timestamps.pop();
        
        std::cout << "check timestamp: " << cur_time << std::endl;

        WeightedIcebreakers icebreakers_waiting;
        std::set<ShipId> targeted;
        // get all arrived in vertices icebreakers sorted by weight
        for (auto &icebreaker: icebreakers) {
            std::vector<ShipId> to_remove;
            auto last_voyage = manager.getCurrentVoyage(icebreaker.id);
            // ВОПРОС: ЗАЧЕМ end_time == 0?
            if (last_voyage.end_time == 0 || last_voyage.end_time <= cur_time) {
                // move caravan to vertex

                if (!last_voyage.end_time == 0)
                    icebreaker.cur_pos = last_voyage.end_point;

                for (auto &ship_id : icebreaker.caravan.ships_id) {
                    if (!last_voyage.end_time == 0)
                        ships[ship_id.id].cur_pos = last_voyage.end_point;

                    // remove arrived ships
                    if (ships[ship_id.id].finish == ships[ship_id.id].cur_pos) {
                        std::cout << "SHIP " << ship_id.id << " ARRIVED!\n";
                        ships_waiting.erase(ship_id);
                        ships_in_caravans.erase(ship_id);
                        to_remove.push_back(ship_id);
                    }

                    // pickup
                    if (icebreaker.to_pickup == ship_id && icebreaker.cur_pos == ships[ship_id.id].cur_pos) {
                        icebreaker.to_pickup = ShipId{}; // now its ok, but can be unsafe later
                        icebreaker.caravan.ships_id.insert(ship_id);
                        ships_in_caravans.insert(ship_id);
                    }
                }

                for (auto &ship_id: to_remove)
                    icebreaker.caravan.ships_id.erase(ship_id);

                // need to process this icebreaker
                icebreakers_waiting[icebreaker.id] = weightIcebreaker(icebreakers[icebreaker.id.id], manager, ships_in_caravans);
            }
        }

        // calc max waiting time
        double cur_max_wait = 0;
        for (auto& [ship_id, _] : ships_waiting) {
            double diff = difftime(cur_time, ships[ship_id.id].voyage_start_date);
            if (diff > cur_max_wait)
                cur_max_wait = diff;
        }

        // get all waiting ships sorted by weight
        for (const auto& ship: ships)
            if (!ships_waiting.count(ship.id) && ship.voyage_start_date <= cur_time)
                if (manager.getCurrentVoyage(ship.id).end_time == 0)
                    ships_waiting[ship.id] = weightShipAlone(ship, cur_time, max_speed, cur_max_wait);

        // make decisions by priority        
        for (const auto& [icebreaker_id, _]: icebreakers_waiting) {
            auto& icebreaker = icebreakers[icebreaker_id.id];
            WeightedShips ships4icebreaker = ships_waiting;
            for (auto& [ship_id, ship_score]: ships4icebreaker) {
                ship_score += weightShipForIcebreaker(icebreaker, ships[ship_id.id], manager);
            }

            Voyage decision;
            decision.end_time = 0;
            bool choose_pickup = false;

            // check all ships by priority
            for (const auto& [best_ship_id, best_ship_score]: ships_waiting) {
                const Ship& cur_ship = ships[best_ship_id.id];
                float best_score = best_ship_score;

                if (ships_in_caravans.count(cur_ship.id))
                    continue;
            
                if ((best_score > score_threshold || icebreaker.caravan.ships_id.empty()) && // good score or empty caravan
                    icebreaker.caravan.ships_id.size() < MAX_SHIPS && // check limit on caravan size
                    !targeted.count(best_ship_id)) // we dont want to target one ship by different icebreakers in one iteration
                { 
                    if (icebreaker.cur_pos == cur_ship.cur_pos) {
                        icebreaker.caravan.ships_id.insert(best_ship_id);
                        ships_in_caravans.insert(best_ship_id);
                    }
                    else {// move to the chosen ship
                        decision = manager.sail2point(icebreaker, cur_ship.cur_pos, cur_time); 
                        icebreaker.to_pickup = best_ship_id;
                        targeted.insert(best_ship_id);
                        choose_pickup = true;
                    }
                    break;
                }
            }
            if (!choose_pickup && !icebreaker.caravan.ships_id.empty()) // move to drop-off points
                decision = manager.sail2depots(icebreaker, cur_time); // TODO: we get here in non-moving pickup decision, need to process it better
            
            if (!decision.end_time == 0) {
                timestamps.push(decision.end_time);
                std::cout << "icebr " << size_t(icebreaker.id.id);
                if (!icebreaker.caravan.ships_id.empty()) {
                    std::cout << " + ships";
                    for (auto &sh_id: icebreaker.caravan.ships_id)
                        std::cout << " " << sh_id.id;
                }
                std::cout << ": " << decision.start_point << "->" << decision.end_point << "\n";
                res.push_back({icebreaker.caravan, decision, icebreaker.id});
            }
        }
    }

    return res;
}