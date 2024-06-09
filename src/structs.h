#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <unordered_map>

#include "graph.h"

// Input data types

#define MAX_SHIPS 3

enum class IceClass {
    kNoIceClass,
    kArc4,
    kArc5,
    kArc7,
    kArc9
};

inline IceClass FromStringToIceClass(const std::string& ice_class_str) {
    if (ice_class_str == "Нет") {
        return IceClass::kNoIceClass;
    } else if (ice_class_str == "Arc 4") {
        return IceClass::kArc4;
    } else if (ice_class_str == "Arc 5") {
        return IceClass::kArc5;
    } else if (ice_class_str == "Arc 7") {
        return IceClass::kArc7;
    } else if (ice_class_str == "Arc 9") {
        return IceClass::kArc9;
    }

    throw std::logic_error("unknown ice class: " + ice_class_str);
}

using Date = time_t;

typedef std::vector<std::vector<float>> IceGrid;

typedef uint32_t VertID; // id of vertex in graph

struct Voyage { // path between 2 vertices
    VertID start_point, end_point;
    Date start_time = 0, end_time = 0;
};

typedef uint32_t BoatID;

typedef std::vector<std::pair<std::vector<BoatID>, Voyage>> Schedule; // result of algorithm - caravan + path

// Boat types

struct BoatInfo {
    BoatID id;
    std::string name;
    float knot_speed; // on clean water!
    IceClass ice_class;
    VertID cur_pos;
};

struct Ship: public BoatInfo {
    int voyage_start_date;
    VertID finish;
};

struct Icebreaker: public BoatInfo {
    std::vector<BoatID> caravan;
};

using Ships = std::vector<Ship>;
using Icebreakers = std::vector<Icebreaker>;

// Path types

typedef std::vector<VertID> Path; // set of vertices
typedef std::vector<std::vector<Path>> Routes; // matrix of full path between every pair of vertices


class PathManager {
private:
    Graph graph;
    IceGrid ice_grid;
    Routes routes;
    std::unordered_map<BoatID, Voyage> current_voyage;

public:
    PathManager(Graph &graph, IceGrid &ice_grid) :
        graph(std::move(graph)), ice_grid(std::move(ice_grid)) // или как там хз
    {
        // TODO: weight edges + floyd to fill routes
    }
    // build path to point, return next step, update current_route for all boats in caravan
    Voyage sail2point(Icebreaker &icebreaker, VertID point) {
        return {}; // TODO
    }
    // build path to all icebreaker's caravan final points, return next step, update current_route
    Voyage sail2depots(Icebreaker &icebreaker) {
        return {}; // TODO
    }
    Voyage getCurrentVoyage(BoatID boat) {
        if (current_voyage.count(boat))
            return current_voyage[boat];
        return {};
    }
};

