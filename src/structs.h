#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <unordered_map>
#include <memory>
#include <set>

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

typedef uint32_t VertID; // id of vertex in graph

struct Voyage { // path between 2 vertices
    VertID start_point, end_point;
    Date start_time = 0, end_time = 0;
};

typedef uint32_t BoatID;

typedef std::set<BoatID> Caravan;

typedef std::vector<std::pair<Caravan, Voyage>> Schedule; // result of algorithm - caravan + path

// Boat types

struct BoatInfo {
    BoatID id;
    std::string name;
    double knot_speed; // on clean water!
    IceClass ice_class;
    VertID cur_pos;
};

struct Ship: public BoatInfo {
    int voyage_start_date;
    VertID finish;
};

struct Icebreaker: public BoatInfo {
    Caravan caravan;
    BoatID to_pickup = 0;
};

using Ships = std::vector<Ship>;
using ShipsPtr = std::shared_ptr<Ships>;
using Icebreakers = std::vector<Icebreaker>;
using IcebreakersPtr = std::shared_ptr<Icebreakers>;

using IceGrid = std::vector<std::vector<double>>;
