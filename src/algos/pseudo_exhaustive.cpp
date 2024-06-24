#include "algos.h"

#include <set>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <numeric>
#include <algorithm>

struct Event {
    Days time;
    bool application = false;
    ScheduleAtom step;

    Event(const Days &t): time(t), application(true) {}
    Event(const Days &t, ScheduleAtom &s): time(t), step(s) {}
};

bool operator<(const Event& lhs, const Event& rhs)
{
    return lhs.time > rhs.time; // ok...
}

namespace {

typedef std::priority_queue<Event> DateQueue;

DateQueue collectVoyagesStarts(Ships &ships, Days cur_time = 0) {
    DateQueue timestamps;
    for (auto& ship : ships) {
        if (cur_time < ship.voyage_start_date) {
            // std::cout << "voyage_start_date: " << ship.voyage_start_date << std::endl;
            timestamps.push(Event(ship.voyage_start_date));
        }
    }
    return timestamps;
}

std::vector<Voyage> checkPathAlone(Ship &ship, PathManager &pm) {
    auto alone_time = pm.TimeToArriveAlone(ship, ship.cur_pos, {ship.finish});
    if (alone_time < 100) {
        auto voyages = pm.GetShortestPathAlone(ship, ship.cur_pos, {ship.finish});
        float alone2 = 0;
        for (auto &v: voyages) {
            alone2 += v.end_time - v.start_time;
        }
        std::cout << alone_time << " " << alone2 << "\n";

        // auto provodka_time = pm.TimeToArriveUnderFakeProvodka(ship, pm.icebreakers->at(0), ship.cur_pos, ship.finish);
        // if (alone_time <= provodka_time * 5) {
        // ship can finish sail alone faster
        return pm.GetShortestPathAlone(ship, ship.cur_pos, ship.finish);
        // }
    }
    return {};
}

bool checkShipForCaravan(const Ship &ship, Caravan &caravan, PathManager &pm) {
    if (!caravan.icebreaker_id)
        return true;

    std::vector<VertID> depots;
    for (auto &ship_id: caravan.ships_id)
        depots.push_back(pm.ships->at(ship_id.id).finish);
    depots.push_back(ship.finish);
    if (!depots.empty()) {
        auto [nearest, _] = pm.GetNearestVertex(ship.cur_pos, pm.icebreakers->at(caravan.icebreaker_id->id), depots);
        if (pm.TimeToArriveUnderFakeProvodka(ship, pm.icebreakers->at(caravan.icebreaker_id->id), ship.cur_pos, nearest) > 1000)
            return false;
    }
    return true;
}

}


Schedule algos::pseudo_exhaustive(PathManager &manager, double *sum_res) {
    Ships &ships = *manager.ships;
    Icebreakers &icebreakers = *manager.icebreakers;
    std::unordered_map<ShipId, float> wait_time;
    std::unordered_map<ShipId, float> path_start;
    std::unordered_map<ShipId, float> path_time;
    std::set<ShipId> solo;

    // std::unordered_map<IcebreakerId, std::vector<Voyages>>

    Schedule final_res;
    
    std::unordered_map<IcebreakerId, ScheduleAtom> last_route;
    std::map<IcebreakerId, Caravan> caravans;
    for (auto &icebr: icebreakers) {
        caravans[icebr.id] = Caravan();
        caravans[icebr.id].icebreaker_id = icebr.id;
    }

    // collect all future voyages start time
    DateQueue timestamps = collectVoyagesStarts(ships);

    std::set<ShipId> ships_waiting; // not arrived to depot: waiting + in travel

    std::vector<IcebreakerId> icebreakers_id;
    for (auto &icebreaker: icebreakers) {
        icebreakers_id.push_back(icebreaker.id);
    }

    // Mail loop
    while (!timestamps.empty()) {
        // get next timestamp
        manager.cur_time = timestamps.top().time;
        bool main_event = timestamps.top().application;
        auto next_step = timestamps.top().step;

        timestamps.pop();
        Days next_timepoint = timestamps.top().time;
        if (manager.cur_time >= 44622)
            int a  = 0;

        std::cout << "check timestamp: " << manager.cur_time << std::endl;

        if (!main_event) { // common event
            IcebreakerId icbr_id = *next_step.caravan.icebreaker_id;
            if (last_route.count(icbr_id)) {
                auto cur_step = last_route[icbr_id];
                if (!caravans.count(icbr_id))
                    caravans[icbr_id] = {{}, icbr_id};                
                
                // changes in caravan
                if (caravans[icbr_id].ships_id != cur_step.caravan.ships_id) {
                    // pickup
                    std::vector<ShipId> new_ships;
                    std::set_difference(cur_step.caravan.ships_id.begin(), cur_step.caravan.ships_id.end(),
                        caravans[icbr_id].ships_id.begin(), caravans[icbr_id].ships_id.end(), std::back_inserter(new_ships));
                    if (!new_ships.empty())
                        for (auto &ship: new_ships) {
                            if (ships[ship.id].cur_pos != icebreakers[icbr_id.id].cur_pos)
                                throw std::runtime_error("Can't pickup :(\n");

                            wait_time[ship] = manager.cur_time - ships[ship.id].voyage_start_date;
                            path_start[ship] = manager.cur_time;
                        }
                    
                    // arrive
                    std::vector<ShipId> arrived_ships;
                    std::set_difference(caravans[icbr_id].ships_id.begin(), caravans[icbr_id].ships_id.end(),
                        cur_step.caravan.ships_id.begin(), cur_step.caravan.ships_id.end(), std::back_inserter(arrived_ships));
                    if (!arrived_ships.empty()) {
                        for (auto &ship: arrived_ships) {
                            if (ships[ship.id].cur_pos != ships[ship.id].finish)
                                throw std::runtime_error("Can't finish here :(\n");
                            path_time[ship.id] = manager.cur_time - path_start[ship.id];
                        }
                        main_event = true;
                    }
                    
                    caravans[icbr_id].ships_id = cur_step.caravan.ships_id;
                }

                // move
                std::cout << "[" << icbr_id.id << "]" << CaravanToString(caravans[icbr_id].ships_id) << ": " << cur_step.edge_voyage.start_point << " " << cur_step.edge_voyage.end_point << " " << cur_step.edge_voyage.start_time << ";" << cur_step.edge_voyage.end_time << "\n";
                icebreakers[caravans[icbr_id].icebreaker_id->id].cur_pos = cur_step.edge_voyage.end_point;
                for (auto &ship_id: caravans[icbr_id].ships_id)
                    ships[ship_id.id].cur_pos = cur_step.edge_voyage.end_point;

                final_res.push_back(cur_step);
            }

            last_route[icbr_id] = next_step;
            manager.icebreaker_to_voyage[icbr_id] = next_step.edge_voyage;
            for (auto &ship: caravans[icbr_id].ships_id) {
                manager.ship_to_voyage[ship.id] = next_step.edge_voyage;
            }
        }

        // full update
        if (main_event) {
            for (auto &[_, car]: caravans)
                car.ships_id.clear();
            last_route.clear();
            while (!timestamps.empty())
                timestamps.pop();
            timestamps = collectVoyagesStarts(ships, manager.cur_time);

            for (const auto& ship: ships)
                if (ship.voyage_start_date <= manager.cur_time && !path_time.count(ship.id))
                    ships_waiting.insert(ship.id);
            
            // all possible caravans
            std::vector<ShipId> wait_vec(ships_waiting.begin(), ships_waiting.end());
            std::vector<std::set<ShipId>> ships_combinations;
            for (int comb_size = manager.GetMaxShipsInCaravan(); comb_size > 0; comb_size--) {
                auto cur_combinations = getAllCombinations<ShipId>(wait_vec, comb_size);
                ships_combinations.insert(ships_combinations.begin(), cur_combinations.begin(), cur_combinations.end());
            }
            for (int i = 0; i < icebreakers.size() - 1; i++)
                ships_combinations.push_back({});

            // pairing with icebreakers
            std::vector<std::vector<float>> icebreaker2caravan_weights(icebreakers.size()); // !TODO index to icebreaker id
            for (int i = 0; i < icebreakers.size(); i++) {
                auto &icebreaker = icebreakers[i];
                icebreaker2caravan_weights[i].resize(ships_combinations.size());
                for (int j = 0; j < ships_combinations.size(); j++) {
                    Caravan caravan;
                    caravan.icebreaker_id = icebreaker.id;
                    caravan.ships_id = std::set<ShipId>(ships_combinations[j].begin(), ships_combinations[j].end());
                    auto [time, _] = manager.TimeToSail(caravan);
                    if (ships_combinations[j].size() == 0)
                        icebreaker2caravan_weights[i][j] = 1000;
                    else
                        icebreaker2caravan_weights[i][j] = time / ships_combinations[j].size();
                        //(manager.GetMaxShipsInCaravan() - ships_combinations[j].size()) * 10; // penalty for empty space
                }
            }

            // all incompatible combintaions for ships
            std::vector<std::pair<size_t, size_t>> incompatible;
            incompatible.reserve(ships_combinations.size() / 3);
            for (int i = 0; i < ships_combinations.size(); i++) {
                for (int j = i + 1; j < ships_combinations.size(); j++) {
                    std::vector<ShipId> result;
                    std::set_intersection(ships_combinations[i].begin(), ships_combinations[i].end(),
                                            ships_combinations[j].begin(), ships_combinations[j].end(),
                                            std::back_inserter(result));
                    if (!result.empty())
                        incompatible.push_back({i, j});
                }
            }
            
            // optimization - choose best pairs caravan-icebreaker
            auto decision = AssignmentCP(icebreaker2caravan_weights, incompatible);
            // ships_combinations.clear();
            icebreaker2caravan_weights.clear();
            incompatible.clear();

            // process decision
            Caravans final_decisions;
            for (auto &[icebr_id, comb_id]: decision) {
                Caravan new_caravan;
                new_caravan.icebreaker_id = icebr_id;
                new_caravan.ships_id = ships_combinations[comb_id];
                if (!new_caravan.ships_id.empty()) {
                    // get path
                    auto [time, points] = manager.TimeToSail(new_caravan);
                    auto [caravan_schedule, solo_schedules] = manager.SailPath(icebreakers[icebr_id], points);
                    
                    for (auto &atom: caravan_schedule) {
                        std::cout << atom.caravan.icebreaker_id->id << " " << atom.edge_voyage.start_point << "->" << atom.edge_voyage.end_point << " " << atom.edge_voyage.start_time << ";" << atom.edge_voyage.end_time << "\n";
                        timestamps.push(Event(atom.edge_voyage.start_time, atom));
                    }
                }
            }
        }

        // log
        // if (decision.end_time != 0) {
        //     timestamps.push(decision.end_time);
        //     std::cout << "[" << size_t(icebreaker.id.id) << "]";
        //     if (!cur_caravan.ships_id.empty())
        //         std::cout << CaravanToString(cur_caravan.ships_id);
        //     std::cout << ": " << decision.start_point << "->" << decision.end_point << "\n";
        //     res.push_back({cur_caravan, decision});
        // }
    }

    std::cout << "Solo:\n";
    for (auto &ship_id: solo)
        std::cout << ship_id.id << " ";
    std::cout << "\n";
    std::cout << "Waiting time:\n";
    float sum = 0;
    for (auto &ship: ships) {
        if (!path_time.count(ship.id))
            std::cout << "WRONG! ship " << ship.name << "(" << ship.id.id << ") didn't arrive to depot\n";
        else {
            std::cout << "\t" << ship.name << "(" << ship.id.id << "): " << wait_time[ship.id] << " + " << path_time[ship.id] << "\n";
            sum += wait_time[ship.id] + path_time[ship.id];
        }
    }
    std::cout << "SUM: " << sum << "\n";
    *sum_res = sum;

    return final_res;
}