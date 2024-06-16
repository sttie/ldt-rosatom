#include "algos.h"

#include <set>
#include <queue>
#include <iostream>
#include <algorithm>

const double score_threshold = 0.3;

float weightShipAlone(const Ship &ship, const Days &cur_time, float max_speed, double cur_max_waiting_time) {
    float
        w1 = 0.0, speed_coef = ship.speed / max_speed,
        w2 = 0.1, waiting_coef = cur_max_waiting_time != 0 ? (cur_time - ship.voyage_start_date) / cur_max_waiting_time : 0;
    return w1 * speed_coef + w2 * waiting_coef;
}

float weightShipForIcebreaker(const Icebreaker &icebreaker, const Ship &ship, const PathManager &pm, const Caravan &caravan) {
    float w3 = 0.9;

    // calculate how much this little maneuver is gonna cost us
    float longination_coef = 0;
    std::vector<VertID> cur_route;
    for (const auto& ship_id : caravan.ships_id)
        cur_route.push_back(pm.ships.get()->at(ship_id.id).finish);

    if (!cur_route.empty()) {
        auto [nearest, _] = pm.GetNearestVertex(ship.cur_pos, ship, cur_route);
        // if (std::isinf(pm.TimeToArriveAlone(ship, ship.cur_pos, nearest)))
        //     return 0;
        double dist1 = pm.PathDistance(icebreaker.cur_pos, icebreaker, cur_route);
        double dist_to_pickup = pm.PathDistance(icebreaker.cur_pos, icebreaker, {ship.cur_pos});
        cur_route.push_back(ship.finish);
        double dist_to_finish = pm.PathDistance(icebreaker.cur_pos, icebreaker, cur_route);
        longination_coef = dist1 / (dist_to_pickup + dist_to_finish); // smaller/bigger
    }

    return w3 * longination_coef;// + w4 * dist_coef;
}

float weightIcebreaker(const Icebreaker &icebreaker, const Caravan &caravan, const PathManager &pm, const std::set<ShipId> &in_caravans) {
    float caravan_coef = (caravan.ships_id.size()) / MAX_SHIPS;

    std::vector<VertID> waiting_positions;
    std::vector<ShipId> waiting_ships;
    for (const auto &ship: *pm.ships) {
        if (!in_caravans.count(ship.id) && ship.voyage_start_date <= pm.cur_time) {
            waiting_ships.push_back(ship.id);
            waiting_positions.push_back(ship.cur_pos);
        }
    }
    if (!waiting_positions.empty()) {
        auto [vertex, distance] = pm.GetNearestVertex(icebreaker.cur_pos, icebreaker, waiting_positions);
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

void checkShipPossibilities(Caravan &caravan, VertID start, VertID end,
    PathManager &pm, std::set<ShipId> &ships_in_caravans, bool to_depots = false)
{
    for (auto &ship_id: caravan.ships_id) {
        Ship &ship = pm.ships->at(ship_id.id);
        auto res = pm.TimeToArriveUnderFakeProvodka(ship, start, end);
        if (std::isinf(res)) {
            // drop this ship, it can't pass ice
            caravan.ships_id.erase(ship_id);
            ships_in_caravans.erase(ship_id);
            pm.ship_to_voyage[ship_id.id] = Voyage{};
        }

        // if (pm.TimeToArriveAlone(ship, ship.cur_pos, {ship.finish}) <
        //     pm.TimeToArriveUnderFakeProvodka(ship, ship.cur_pos, ship.finish) * 1.5) {
        //     // ship can finish sail alone faster
            
        // }
    }
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
        max_speed = std::max(ship.speed, max_speed);
    
    // strart caravans - empty icebreakers
    Caravans caravans;
    std::map<IcebreakerId, size_t> icebreaker2caravan;
    for (auto &icebreaker: icebreakers) {
        Caravan new_caravan;
        new_caravan.icebreaker_id = icebreaker.id;
        caravans.push_back(new_caravan);
        icebreaker2caravan[icebreaker.id] = caravans.size() - 1;
    }

    std::map<IcebreakerId, ShipId> targeted;

    // Mail loop
    while (!timestamps.empty()) {
        // 0. Get next timestamp
        manager.cur_time = timestamps.top();
        while (!timestamps.empty() && manager.cur_time == timestamps.top())
            timestamps.pop();
        std::cout << "check timestamp: " << manager.cur_time << std::endl;

        // 1. Update positions
        for (auto &caravan: caravans) {
            Voyage last_voyage = manager.getCurrentVoyage(caravan);
            bool
                staying = (last_voyage.end_time == 0),
                voyage_finished = last_voyage.end_time <= manager.cur_time + 1e-5;
            if (!staying && voyage_finished) {
                if (caravan.icebreaker_id)
                    icebreakers[caravan.icebreaker_id->id].cur_pos = last_voyage.end_point;
                for (auto &ship_id: caravan.ships_id)
                    ships[ship_id.id].cur_pos = last_voyage.end_point;
            }
        }

        // 2. Remove arrived ships
        for (auto &caravan: caravans) {
            std::vector<ShipId> to_remove;
            for (auto &ship_id: caravan.ships_id)
                if (ships[ship_id.id].cur_pos == ships[ship_id.id].finish) {
                    std::cout << "SHIP " << ship_id.id << " ARRIVED!\n";
                    wait_time[ship_id.id] = manager.cur_time - ships[ship_id.id].voyage_start_date;
                    ships_in_caravans.erase(ship_id.id);
                    ships_waiting.erase(ship_id);
                    to_remove.push_back(ship_id.id);
                }
            for (auto &ship_id: to_remove)
                caravan.ships_id.erase(ship_id.id);
        }

        // 3. Pickup ships with icebreakers on same vertex
        for (auto &[ship_id, _] : ships_waiting) {
            bool
                staying = (manager.getCurrentVoyage(ship_id).end_time == 0),
                in_caravan = ships_in_caravans.count(ship_id);
            if (staying && !in_caravan)
                for (auto &icebreaker: icebreakers)
                    if (icebreaker.cur_pos == ships[ship_id.id].cur_pos &&
                        caravans[icebreaker2caravan[icebreaker.id.id]].ships_id.size() < MAX_SHIPS)
                    {
                        std::cout << "\t[" << icebreaker.id.id << "] picked_up " << ship_id.id << "\n";
                        caravans[icebreaker2caravan[icebreaker.id.id]].ships_id.insert(ship_id);
                        ships_in_caravans.insert(ship_id);
                    }
        }

        // 4. Sort icebreakers
        WeightedIcebreakers icebreakers_waiting;
        for (auto &icebreaker: icebreakers) {
            auto &cur_caravan = caravans[icebreaker2caravan[icebreaker.id.id]];
            if (manager.getCurrentVoyage(icebreaker.id).end_time <= manager.cur_time + 1e-5) {
                targeted.erase(icebreaker.id);
                icebreakers_waiting[icebreaker.id.id] =
                    weightIcebreaker(icebreakers[icebreaker.id.id], cur_caravan, manager, ships_in_caravans);
            }
        }
        auto sorted_icebreaker = sortMapLess<IcebreakerId, float>(icebreakers_waiting);


        // 5. Make decisions
        
        // 5.0 Calc max waiting time
        double cur_max_wait = 0;
        for (auto& [ship_id, _] : ships_waiting) {
            double diff = manager.cur_time - ships[ship_id.id].voyage_start_date;
            if (diff > cur_max_wait)
                cur_max_wait = diff;
        }
        
        // 5.1 Collect all waiting ships
        for (const auto& ship: ships)
            if (ship.voyage_start_date <= manager.cur_time && !wait_time.count(ship.id))
                if (manager.getCurrentVoyage(ship.id).end_time == 0)
                    ships_waiting[ship.id] = weightShipAlone(ship, manager.cur_time, max_speed, cur_max_wait);
        
        // 5.2 Set destinations
        for (const auto& [icebreaker_id, _]: sorted_icebreaker) {
            auto &icebreaker = icebreakers[icebreaker_id.id];
            auto &cur_caravan = caravans[icebreaker2caravan[icebreaker.id.id]];

            Voyage decision;
            decision.end_time = 0;
            bool choose_pickup = false;

            if (cur_caravan.ships_id.size() < MAX_SHIPS) {
                // Add weights especially for this icebreaker and sort ships
                WeightedShips ships4icebreaker = ships_waiting;
                for (auto& [ship_id, ship_score]: ships4icebreaker)
                    ship_score = 1 - (ship_score + weightShipForIcebreaker(icebreaker, ships[ship_id.id], manager, cur_caravan));
                auto sorted_ships = sortMapLess<ShipId, float>(ships4icebreaker);

                // Collect already targeted ships
                std::set<ShipId> targeted_ships;
                for (auto &[icbr_id, target]: targeted)
                    if (icbr_id.id != icebreaker_id.id)
                        targeted_ships.insert(target);

                // Check all ships by priority
                for (const auto& [best_ship_id, best_ship_score]: sorted_ships) {
                    const Ship& cur_ship = ships[best_ship_id.id];
                    float best_score = best_ship_score;

                    if (ships_in_caravans.count(cur_ship.id))
                        continue;

                    // Decide to pick-up ship
                    if ((best_score < score_threshold || cur_caravan.ships_id.empty()) && // good score or empty caravan
                        cur_caravan.ships_id.size() < MAX_SHIPS && // check limit on caravan size
                        !targeted_ships.count(best_ship_id)) // we dont want to target one ship by different icebreakers in one iteration
                    {
                        std::cout << "\t[" << icebreaker.id.id << "] going for ship " << cur_ship.id.id << "\n";

                        // checkShipPossibilities(cur_caravan, icebreaker.cur_pos, cur_ship.cur_pos, manager, ships_in_caravans);
                        
                        decision = manager.sail2point(icebreaker, cur_caravan, cur_ship.cur_pos); 
                        targeted[icebreaker.id] = best_ship_id;
                        choose_pickup = true;
                        
                        break;
                    }
                }
            }
            
            // Decide to move to drop-off points
            if (!choose_pickup && !cur_caravan.ships_id.empty()) {
                std::cout << "\t[" << icebreaker.id.id << "] sailing to depots\n";

                // std::vector<VertID> drops;
                // for (auto &ship_id: cur_caravan.ships_id)
                //     drops.push_back(ships[ship_id.id].finish);
                // auto [next, _] = manager.GetNearestVertex(icebreaker.cur_pos, icebreaker, drops);

                // checkShipPossibilities(cur_caravan, icebreaker.cur_pos, next, manager, ships_in_caravans);
                decision = manager.sail2depots(icebreaker, cur_caravan);
            }

            // log
            if (decision.end_time != 0) {
                timestamps.push(decision.end_time);
                std::cout << "[" << size_t(icebreaker.id.id) << "]";
                if (!cur_caravan.ships_id.empty())
                    CaravanToString(cur_caravan.ships_id);
                std::cout << ": " << decision.start_point << "->" << decision.end_point << "\n";
                res.push_back({cur_caravan, decision});
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