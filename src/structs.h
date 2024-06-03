#pragma once

#include <string>
#include <stdexcept>
#include <chrono>

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

using Date = std::chrono::system_clock::time_point;

struct Ship {
    std::string name;
    IceClass ice_class;
    double knot_speed;          // on clean water!
    std::string departure;      // maybe it will be just a reference to graph's vertex
    std::string destination;    // maybe it will be just a reference to graph's vertex
    int voyage_start_date;      // wtf is this?
};

struct Icebreaker {
    std::string name;
    double knot_speed;
    IceClass ice_class;
    std::string departure;
};
