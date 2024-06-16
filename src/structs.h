#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <unordered_map>
#include <memory>
#include <set>
#include <map>
#include <array>
#include <boost/optional.hpp>

// Input data types

#define MAX_SHIPS 3

enum class IceClass {
    kNoIceClass,
    kArc1,
    kArc2,
    kArc3,
    kArc4,
    kArc5,
    kArc6,
    kArc7,
    kArc8,
    kArc9
};

inline IceClass FromStringToIceClass(const std::string& ice_class_str) {
    if (ice_class_str == "Нет") {
        return IceClass::kNoIceClass;
    } else if (ice_class_str == "Arc 1") {
        return IceClass::kArc1;
    } else if (ice_class_str == "Arc 2") {
        return IceClass::kArc2;
    } else if (ice_class_str == "Arc 3") {
        return IceClass::kArc3;
    } else if (ice_class_str == "Arc 4") {
        return IceClass::kArc4;
    } else if (ice_class_str == "Arc 5") {
        return IceClass::kArc5;
    } else if (ice_class_str == "Arc 6") {
        return IceClass::kArc6;
    } else if (ice_class_str == "Arc 7") {
        return IceClass::kArc7;
    } else if (ice_class_str == "Arc 8") {
        return IceClass::kArc8;
    } else if (ice_class_str == "Arc 9") {
        return IceClass::kArc9;
    }

    throw std::logic_error("unknown ice class: " + ice_class_str);
}

constexpr size_t ICE_WEEKS_AMOUNT = 1;

struct IceGrid {
    std::vector<std::vector<float>> lon;
    std::vector<std::vector<float>> lat;
    
    using Ice = std::vector<std::vector<float>>;
    std::array<Ice, ICE_WEEKS_AMOUNT> weekly_ice;
};

using Days = float;

using VertID = size_t; // id of vertex in graph

struct PointId {
    VertID id;
};

struct GraphPoint {
    PointId point_id;
    float latitude;
    float longitude;
    std::string point_name;
};

using GraphPointsInfo = std::vector<GraphPoint>;

struct Voyage { // path between 2 vertices
    VertID start_point, end_point;
    Days start_time = 0, end_time = 0;
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
    boost::optional<IcebreakerId> icebreaker_id;
};

struct Ship {
    ShipId id;
    std::string name;
    float speed; // on clean water!
    IceClass ice_class;
    VertID cur_pos;
    Days voyage_start_date;
    VertID finish;
};

struct Icebreaker {
    IcebreakerId id;
    std::string name;
    float speed; // on clean water!
    IceClass ice_class;
    VertID cur_pos;
};

using Ships = std::vector<Ship>;
using ShipsPtr = std::shared_ptr<Ships>;
using Icebreakers = std::vector<Icebreaker>;
using IcebreakersPtr = std::shared_ptr<Icebreakers>;

struct SheculeAtom {
    Caravan caravan;
    Voyage edge_voyage;
};

typedef std::vector<SheculeAtom> Schedule; // result of algorithm - caravan + path

inline std::string CaravanToString(const std::set<ShipId> &caravan) {
    std::string caravan_str = "{";
    for (auto it = caravan.begin(); it != caravan.end(); ++it) {
        caravan_str += std::to_string(it->id);
        if (it != std::prev(caravan.end())) {
            caravan_str += ", ";
        }
    }
    caravan_str += "}";

    return caravan_str;
}


using WeightedShips = std::map<ShipId, float>;
using WeightedIcebreakers = std::map<IcebreakerId, float>;

typedef std::vector<Caravan> Caravans;

struct compCaravans {
    WeightedIcebreakers icebreakers;
    WeightedShips ships;

    bool operator()(const Caravan &a, const Caravan &b) {
        // lonely ships must be parsed before icebreakers
        if (a.icebreaker_id.is_initialized() != b.icebreaker_id.is_initialized())
            return a.icebreaker_id.is_initialized() < b.icebreaker_id.is_initialized();

        // caravans with icebreakers
        if (a.icebreaker_id.is_initialized())
            return icebreakers[a.icebreaker_id->id] < icebreakers[b.icebreaker_id->id];
        
        // lonely ships
        return ships[*a.ships_id.begin()] < ships[*b.ships_id.begin()];
    }
};

typedef std::set<Caravan, compCaravans> SortedCaravans;
