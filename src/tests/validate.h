#pragma once

#include <structs.h>
#include <path.h>

namespace {

void Assert(bool assertion, std::ostringstream&& message) {
    if (!assertion) {
        throw std::runtime_error(message.str());
    }
}

std::vector<std::pair<Voyage, bool>> BuildSortedPathForShip(const Ship& ship, const Schedule& schedule) {
    std::vector<std::pair<Voyage, bool>> path;

    for (const auto& atom : schedule) {
        if (std::find(atom.caravan.ships_id.begin(), atom.caravan.ships_id.end(), ship.id) != atom.caravan.ships_id.end()) {
            path.push_back(std::make_pair(atom.edge_voyage, !atom.caravan.icebreaker_id.is_initialized()));
        }
    }

    std::sort(path.begin(), path.end(), [](const auto& a, const auto& b ) {
        return a.first.start_time < b.first.start_time ||
                (a.first.start_time == b.first.start_time && a.first.end_time < b.first.end_time);
    });

    return path;
}

// std::vector<Voyage> BuildPathForIcebreaker() {
//     return {};
// }

void ValidateShipPath(const Ship& ship, const Schedule& schedule, const PathManager& pm) {
    auto path = BuildSortedPathForShip(ship, schedule);

    // std::ostringstream message;
    Assert(ship.cur_pos == path.front().first.start_point, std::ostringstream{}
           << ship.name << "(" << ship.id.id << "): start position doesn't equal to path's start point");

    auto [current_voyage, alone] = path.front();
    auto start = current_voyage.start_point;
    auto end = current_voyage.end_point;

    for (size_t i = 1; i < path.size(); ++i) {
        Assert(pm.HasEdge(ship, alone, current_voyage.start_time, start, end), std::ostringstream{} << "no edge between start and end");
        Assert(end == path[i].first.start_point, std::ostringstream{} << "no edge between current end and next start");

        current_voyage = path[i].first;
        alone = path[i].second;
        start = current_voyage.start_point;
        end = current_voyage.end_point;
    }

    Assert(end == ship.finish, std::ostringstream{} << "i'm tired");
}

}

void Validate(ShipsPtr ships, const Schedule& schedule, const PathManager& pm) {
    for (const auto& ship : *ships) {
        ValidateShipPath(ship, schedule, pm);
    }
}
