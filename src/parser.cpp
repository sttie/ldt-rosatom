#include "parser.h"

#include "structs.h"

#include <OpenXLSX.hpp>
#include <iostream>
#include <unordered_map>

namespace parser {

namespace {

std::unordered_map<std::string, int> month_to_index = {
    {"January", 0},
    {"Febuary", 1},
    {"March", 2},
    {"April", 3},
    {"May", 4},
    {"June", 5},
    {"July", 6},
    {"August", 7},
    {"September", 8},
    {"October", 9},
    {"November", 10},
    {"December", 11},
};

}

Ships ParseShipsSchedule(const std::string& dataset_path) {
    OpenXLSX::XLDocument doc{dataset_path};
    if (!doc.isOpen()) {
        throw std::runtime_error("unable to open " + dataset_path + " file");
    }
    
    auto wks = doc.workbook().worksheet(1);

    Ships ships;
    for (size_t row = 2;;++row) {
        std::cout << "row: " << row << std::endl;
        if (wks.cell("A" + std::to_string(row)).value().type() == OpenXLSX::XLValueType::Empty) {
            break;
        }

        Ship ship;
        ship.name = wks.cell("A" + std::to_string(row)).value().getString();
        ship.ice_class = FromStringToIceClass(wks.cell("B" + std::to_string(row)).value().getString());

        auto knot_speed_cell = wks.cell("C" + std::to_string(row));
        if (knot_speed_cell.value().type() == OpenXLSX::XLValueType::Integer) {
            ship.knot_speed = static_cast<double>(knot_speed_cell.value().get<int>());
        } else if (knot_speed_cell.value().type() == OpenXLSX::XLValueType::Float) {
            ship.knot_speed = knot_speed_cell.value().get<double>();
        } else {
            throw std::runtime_error("wtf is the type of knot_speed column (ship)?..");
        }

        ship.departure = wks.cell("D" + std::to_string(row)).value().getString();
        ship.destination = wks.cell("E" + std::to_string(row)).value().getString();
        ship.voyage_start_date = wks.cell("F" + std::to_string(row)).value().get<int>();

        // char weekday_str[20], month_str[20]; int monthday = -1, year = -1;
        // sprintf(voyage_start_date.data(), "%s, %s %d, %d", weekday_str, month_str, monthday, year);
    
        // std::tm tm = {  /* .tm_sec  = */ 0,
        //                 /* .tm_min  = */ 0,
        //                 /* .tm_hour = */ 0,
        //                 /* .tm_mday = */ monthday,
        //                 /* .tm_mon  = */ month_to_index[month_str],
        //                 /* .tm_year = */ year - 1900,
        //                 };
        // tm.tm_isdst = -1; // Use DST value from local time zone
        // auto voyage_start_tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        
        ships.push_back(std::move(ship));
    }

    return ships;
}

Icebreakers ParseIcebreakers(const std::string& dataset_path) {
    OpenXLSX::XLDocument doc{dataset_path};
    if (!doc.isOpen()) {
        throw std::runtime_error("unable to open " + dataset_path + " file");
    }

    Icebreakers icebreakers;
    auto wks = doc.workbook().worksheet(1);

    for (size_t row = 47;;++row) {
        if (wks.cell("C" + std::to_string(row)).value().type() == OpenXLSX::XLValueType::Empty) {
            break;
        }

        Icebreaker icebreaker;
        icebreaker.name = wks.cell("C" + std::to_string(row)).value().getString();
        
        auto knot_speed_cell = wks.cell("D" + std::to_string(row));
        if (knot_speed_cell.value().type() == OpenXLSX::XLValueType::Integer) {
            icebreaker.knot_speed = static_cast<double>(knot_speed_cell.value().get<int>());
        } else if (knot_speed_cell.value().type() == OpenXLSX::XLValueType::Float) {
            icebreaker.knot_speed = knot_speed_cell.value().get<double>();
        } else {
            throw std::runtime_error("wtf is the type of knot_speed column (icebreaker)?..");
        }

        icebreaker.ice_class = FromStringToIceClass(wks.cell("E" + std::to_string(row)).value().getString());
        icebreaker.departure = wks.cell("F" + std::to_string(row)).value().getString();

        icebreakers.push_back(std::move(icebreaker));
    }

    return icebreakers;
}

}
