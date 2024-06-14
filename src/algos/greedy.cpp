#include "algos.h"

#include <set>
#include <queue>
#include <iostream>
#include <algorithm>

using WeightedShips = std::map<ShipId, float>; //todo

using WeightedIcebreakers = std::map<IcebreakerId, float>; //todo

const float score_threshold = 0.4;

float weightShipAlone(const Ship &ship, const Days &cur_time, float max_speed, float cur_max_waiting_time) {
    float
        w1 = 0, speed_coef = ship.speed / max_speed,
        w2 = 0.1, waiting_coef = cur_max_waiting_time != 0 ? (cur_time - ship.voyage_start_date) / cur_max_waiting_time : 0;
    return w1 * speed_coef + w2 * waiting_coef;
}

float weightShipForIcebreaker(const Icebreaker &icebreaker, const Ship &ship, const PathManager &pm) {
    float w3 = 0.9;

    // calculate how much this little maneuver is gonna cost us
    float longination_coef = 0;
    std::vector<VertID> cur_route;
    for (const auto& ship_id : icebreaker.caravan.ships_id)
        cur_route.push_back(pm.ships.get()->at(ship_id.id).finish);
    if (!cur_route.empty()) {
        float dist1 = pm.PathDistance(icebreaker.cur_pos, cur_route);
        float dist_to_pickup = pm.PathDistance(icebreaker.cur_pos, {ship.cur_pos});
        cur_route.push_back(ship.finish);
        float dist_to_finish = pm.PathDistance(icebreaker.cur_pos, cur_route);
        longination_coef = dist1 / (dist_to_pickup + dist_to_finish); // smaller/bigger
    }

    return w3 * longination_coef;// + w4 * dist_coef;
}

float weightIcebreaker(const Icebreaker &icebreaker, const PathManager &pm, const std::set<ShipId> &in_caravans) {
    float caravan_coef = (icebreaker.caravan.ships_id.size()) / MAX_SHIPS;

    std::vector<VertID> waiting_positions;
    std::vector<ShipId> waiting_ships;
    for (const auto &ship: *pm.ships) {
        if (!in_caravans.count(ship.id) && ship.voyage_start_date <= pm.cur_time) {
            waiting_ships.push_back(ship.id);
            waiting_positions.push_back(ship.cur_pos);
        }
    }
    if (!waiting_positions.empty()) {
        auto [vertex, distance] = pm.GetNearestVertex(icebreaker.cur_pos, waiting_positions);
        float dist_coef = distance * (0.5 + 0.5 * caravan_coef);
        return dist_coef;
    }
    else
        return 0;
}

typedef std::priority_queue<Days, std::vector<Days>, std::greater<Days>> DateQueue;

DateQueue collectVoyagesStarts(const Ships &ships) {
    DateQueue timestamps;
    for (const auto& ship : ships) {
        std::cout << "voyage_start_date: " << ship.voyage_start_date << std::endl;
        timestamps.push(ship.voyage_start_date);
    }
    return timestamps;
}

Schedule algos::greedy(PathManager &manager) {
    Ships &ships = *manager.ships;
    Icebreakers &icebreakers = *manager.icebreakers;
    std::unordered_map<ShipId, float> wait_time;

    Schedule res;

    // collect all future voyages start time
    DateQueue timestamps = collectVoyagesStarts(ships);

    WeightedShips ships_waiting; // not arrived to depot: waiting + in travel
    std::set<ShipId> ships_in_caravans; // all ships in caravans

    // calc max_speed
    float max_speed = 0;
    for (auto &ship: ships)
        if (ship.speed > max_speed)
            max_speed = ship.speed;

    std::map<IcebreakerId, ShipId> targeted;
    while (!timestamps.empty()) {
        // get next timestamp
        manager.cur_time = timestamps.top();
        while (!timestamps.empty() && manager.cur_time == timestamps.top())
            timestamps.pop();
        
        std::cout << "check timestamp: " << manager.cur_time << std::endl;

        WeightedIcebreakers icebreakers_waiting;
        // get all arrived in vertices icebreakers sorted by weight
        for (size_t i = 0; i < icebreakers.size(); i++) {
            auto &icebreaker = icebreakers[i];
            std::vector<ShipId> to_remove;
            auto last_voyage = manager.getCurrentVoyage(icebreaker.id);
            
            bool
                staying = (last_voyage.end_time == 0),
                voyaged_finished = last_voyage.end_time <= manager.cur_time + 0.01 ;
            
            if (staying || voyaged_finished) {
                // move caravan to vertex

                if (!staying)
                    icebreaker.cur_pos = last_voyage.end_point;

                for (auto &ship_id : icebreaker.caravan.ships_id) {
                    auto &cur_ship = ships[ship_id.id];
                    if (!staying)
                        cur_ship.cur_pos = last_voyage.end_point;

                    // ships arrived to depot
                    if (cur_ship.finish == cur_ship.cur_pos) {
                        std::cout << "SHIP " << ship_id.id << " ARRIVED!\n";
                        wait_time[cur_ship.id] = manager.cur_time - cur_ship.voyage_start_date;
                        ships_waiting.erase(ship_id);
                        ships_in_caravans.erase(ship_id);
                        to_remove.push_back(ship_id);
                    }
                }

                for (auto &ship_id: to_remove)
                    icebreaker.caravan.ships_id.erase(ship_id);

                for (auto &[ship_id, _] : ships_waiting) {
                    // pickup
                    if (icebreaker.caravan.ships_id.size() < MAX_SHIPS &&
                        icebreaker.cur_pos == ships[ship_id.id].cur_pos &&
                        !ships_in_caravans.count(ship_id))
                    {
                        std::cout << "\t[" << icebreaker.id.id << "] picked_up " << ship_id.id << "\n";
                        icebreaker.caravan.ships_id.insert(ship_id);
                        ships_in_caravans.insert(ship_id);
                    }
                }

                targeted.erase(icebreaker.id); // remove old decision
                // need to process this icebreaker
                icebreakers_waiting[icebreaker.id] = weightIcebreaker(icebreakers[icebreaker.id.id], manager, ships_in_caravans);
                // std::cout << "\t[" << icebreaker.id.id << "] " << icebreakers_waiting[icebreaker.id] << "\n";
            }
        }

        // Sort boats by priority
        auto sorted_icebreaker = sortMapLess<IcebreakerId, float>(icebreakers_waiting);

        // calc max waiting time
        float cur_max_wait = 0;
        for (auto& [ship_id, _] : ships_waiting) {
            float diff = manager.cur_time - ships[ship_id.id].voyage_start_date;
            if (diff > cur_max_wait)
                cur_max_wait = diff;
        }

        // get all waiting ships sorted by weight
        for (const auto& ship: ships)
            if (!ships_waiting.count(ship.id) && ship.voyage_start_date <= manager.cur_time)
                if (manager.getCurrentVoyage(ship.id).end_time == 0)
                    ships_waiting[ship.id] = weightShipAlone(ship, manager.cur_time, max_speed, cur_max_wait);
        
        std::cout << "Total ships waiting: " << ships_waiting.size() << "\n";

        // make decisions by priority        
        for (const auto& [icebreaker_id, _]: sorted_icebreaker) {
            auto& icebreaker = icebreakers[icebreaker_id.id];
            WeightedShips ships4icebreaker = ships_waiting;
            for (auto& [ship_id, ship_score]: ships4icebreaker) {
                ship_score = 1 - (ship_score + weightShipForIcebreaker(icebreaker, ships[ship_id.id], manager));
                // std::cout << "\t" << ship_id.id << " " << ship_score << "\n";
            }
            auto sorted_ships = sortMapLess<ShipId, float>(ships4icebreaker);

            Voyage decision;
            decision.end_time = 0;
            bool choose_pickup = false;

            // icebreakers already targeted on this ships
            std::set<ShipId> targeted_ships;
            for (auto &[icbr_id, target]: targeted)
                if (icbr_id.id != icebreaker_id.id)
                    targeted_ships.insert(target);

            // check all ships by priority
            for (const auto& [best_ship_id, best_ship_score]: sorted_ships) {
                const Ship& cur_ship = ships[best_ship_id.id];
                float best_score = best_ship_score;

                if (ships_in_caravans.count(cur_ship.id))
                    continue;
            
                if ((best_score < score_threshold || icebreaker.caravan.ships_id.empty()) && // good score or empty caravan
                    icebreaker.caravan.ships_id.size() < MAX_SHIPS && // check limit on caravan size
                    !targeted_ships.count(best_ship_id)) // we dont want to target one ship by different icebreakers in one iteration
                {
                    std::cout << "\t[" << icebreaker.id.id << "] going for ship " << cur_ship.id.id << "\n";
                    decision = manager.sail2point(icebreaker, cur_ship.cur_pos); 
                    targeted[icebreaker.id] = best_ship_id;
                    choose_pickup = true;
                    
                    break;
                }
            }
            if (!choose_pickup && !icebreaker.caravan.ships_id.empty()) { // move to drop-off points
                std::cout << "\t[" << icebreaker.id.id << "] sailing to depots\n";
                decision = manager.sail2depots(icebreaker);
            }

            if (decision.end_time != 0) {
                timestamps.push(decision.end_time);
                std::cout << "[" << size_t(icebreaker.id.id) << "]";
                if (!icebreaker.caravan.ships_id.empty()) {
                    std::cout << " {";
                    bool first = true;
                    for (auto &sh_id: icebreaker.caravan.ships_id) {
                        if (!first)
                            std::cout << " ";
                        else
                            first = false;
                        std::cout << sh_id.id;
                    }
                    std::cout << "}";
                }
                std::cout << ": " << decision.start_point << "->" << decision.end_point << "\n";
                res.push_back({icebreaker.caravan, decision, icebreaker.id});
            }
        }
    }

    std::cout << "Waiting time:\n";
    float sum = 0;
    for (auto &ship: ships) {
        if (!wait_time.count(ship.id))
            std::cout << "WRONG! ship " << ship.name << "(" << ship.id.id << ") didn't arrive to depot\n";
        else {
            std::cout << "\t" << ship.name << "(" << ship.id.id << "): " << wait_time[ship.id] << "\n";
            sum += wait_time[ship.id];
        }
    }
    std::cout << "SUM: " << sum << "\n";

    return res;
}