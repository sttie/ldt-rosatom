#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <unordered_map>
#include <memory>
#include <set>
#include <array>

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

constexpr size_t ICE_WEEKS_AMOUNT = 1;

struct IceGrid {
    std::vector<std::vector<double>> lon;
    std::vector<std::vector<double>> lat;
    
    using Ice = std::vector<std::vector<double>>;
    std::array<Ice, ICE_WEEKS_AMOUNT> weekly_ice;
};

using Date = time_t;

using VertID = size_t; // id of vertex in graph

struct PointId {
    VertID id;
};

struct GraphPoint {
    PointId point_id;
    double latitude;
    double longitude;
    std::string point_name;
};

using GraphPointsInfo = std::vector<GraphPoint>;

struct Voyage { // path between 2 vertices
    VertID start_point, end_point;
    Date start_time = 0, end_time = 0;
};

// Boat types

struct ShipId {
    size_t id;

    ShipId(size_t id = 0)
        : id(id)
    {
    }
    ShipId(const ShipId& other)
        : id(other.id)
    {}

    bool operator<(const ShipId& other) const {
        return id < other.id;
    }

    bool operator==(const ShipId& other) const {
        return id == other.id;
    }
};

template <>
struct std::hash<ShipId> {
    std::size_t operator()(const ShipId& ship_id) const {
        return std::hash<size_t>{}(ship_id.id);
    }
};

struct IcebreakerId {
    size_t id = 0;

    IcebreakerId(size_t id = 0)
        : id(id)
    {
    }
    IcebreakerId(const IcebreakerId& other)
        : id(other.id)
    {}

    bool operator<(const IcebreakerId& other) const {
        return id < other.id;
    }

    bool operator==(const IcebreakerId& other) const {
        return id == other.id;
    }
};

template <>
struct std::hash<IcebreakerId> {
    std::size_t operator()(const IcebreakerId& icebreaker_id) const {
        return std::hash<size_t>{}(icebreaker_id.id);
    }
};


struct Caravan {
    std::set<ShipId> ships_id;
    IcebreakerId icebreaker_id;
};

struct Ship {
    ShipId id;
    std::string name;
    double knot_speed; // on clean water!
    IceClass ice_class;
    VertID cur_pos;
    int voyage_start_date;
    VertID finish;
};

struct Icebreaker {
    IcebreakerId id;
    std::string name;
    double knot_speed; // on clean water!
    IceClass ice_class;
    VertID cur_pos;
    Caravan caravan;
    ShipId to_pickup;
};

using Ships = std::vector<Ship>;
using ShipsPtr = std::shared_ptr<Ships>;
using Icebreakers = std::vector<Icebreaker>;
using IcebreakersPtr = std::shared_ptr<Icebreakers>;

typedef std::vector<std::pair<Caravan, Voyage>> Schedule; // result of algorithm - caravan + path

inline std::string CaravanToString(const Caravan& caravan) {
    std::string caravan_str = "{";
    for (auto it = caravan.ships_id.begin(); it != caravan.ships_id.end(); ++it) {
        caravan_str += std::to_string(it->id);
        if (it != std::prev(caravan.ships_id.end())) {
            caravan_str += ", ";
        }
    }
    caravan_str += "}";

    return caravan_str;
}
