#include "parser.h"

#include "structs.h"

#include <OpenXLSX.hpp>
#include <iostream>
#include <array>
#include <string>
#include <unordered_map>
#include <boost/algorithm/string.hpp>

#include <nlohmann/json.hpp>
#include <fstream>

#include <limits>

namespace parser {

namespace {

constexpr size_t ICE_MAP_ROWS = 268;
constexpr size_t ICE_MAP_COLUMNS = 217;

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

VertID getVertID(const std::string& point_name, const GraphPointsInfo& graph_points_info) {
    auto it = std::find_if(graph_points_info.begin(), graph_points_info.end(),
                           [&point_name](const auto& point) {
                               return boost::algorithm::to_lower_copy(point.point_name) == boost::algorithm::to_lower_copy(point_name);
                           });
    if (it == graph_points_info.end()) {
        throw std::runtime_error("unable to find graph point with name" + point_name);
    }

    return it->point_id.id;
}

std::array<float, 3> GetShipIceDebuff(int ice_type) {
    if (ice_type == 0) {
        return {1.0f, 1.0f, 1.0f};
    } else if (ice_type == 1) {
        return {0.0f, 0.0f, 0.6f};
    } else if (ice_type == 2) {
        return {0.0f, 0.0f, 0.0f};
    } else if (ice_type == 3) {
        return {0.0f, 0.0f, 0.0f};
    }

    throw std::runtime_error("invalid ice_type: " + std::to_string(ice_type));
}

std::array<float, 4> GetIcebreakerSpeed(int ice_type, const std::vector<Icebreaker>& icebreakers) {
    if (ice_type == 0) {
        return {icebreakers[0].speed, icebreakers[1].speed, icebreakers[2].speed, icebreakers[3].speed};
    } else if (ice_type == 1) {
        return {19.0f * 1852.0f * 24.0f, 19.0f * 1852.0f * 24.0f, 19.0f * 0.9f * 1852.0f * 24.0f, 19.0f * 0.9f * 1852.0f * 24.0f};
    } else if (ice_type == 2) {
        return {14.0f * 1852.0f * 24.0f, 14.0f * 1852.0f * 24.0f, 14.0f * 0.75f * 1852.0f * 24.0f, 14.0f * 0.75f * 1852.0f * 24.0f};
    } else if (ice_type == 3) {
        return {0.0f * 1852.0f * 24.0f, 0.0f * 1852.0f * 24.0f, 0.0f * 1852.0f * 24.0f, 0.f * 1852.0f * 24.0f};
    }

    throw std::runtime_error("invalid ice_type: " + std::to_string(ice_type));
}

}


GraphPointsInfo ParseGraphPointsFromExcel(const std::string& graph_filepath) {
    OpenXLSX::XLDocument doc{graph_filepath};
    if (!doc.isOpen()) {
        throw std::runtime_error("unable to open " + graph_filepath + " file");
    }
    auto wks = doc.workbook().worksheet(1);

    GraphPointsInfo points_info;
    for (size_t row = 2;; ++row) {
        if (wks.cell("A" + std::to_string(row)).value().type() == OpenXLSX::XLValueType::Empty) {
            break;
        }

        GraphPoint point;

        point.point_id.id = wks.cell("A" + std::to_string(row)).value().get<size_t>();
        
        auto latitude_cell = wks.cell("B" + std::to_string(row));
        if (latitude_cell.value().type() == OpenXLSX::XLValueType::Integer) {
            point.latitude = static_cast<float>(latitude_cell.value().get<int>());
        } else if (latitude_cell.value().type() == OpenXLSX::XLValueType::Float) {
            point.latitude = latitude_cell.value().get<float>();
        } else {
            throw std::runtime_error("wtf is the type of latitude column (icebreaker)?..");
        }

        auto longitude_cell = wks.cell("C" + std::to_string(row));
        if (longitude_cell.value().type() == OpenXLSX::XLValueType::Integer) {
            point.longitude = static_cast<float>(longitude_cell.value().get<int>());
        } else if (longitude_cell.value().type() == OpenXLSX::XLValueType::Float) {
            point.longitude = longitude_cell.value().get<float>();
        } else {
            throw std::runtime_error("wtf is the type of knot_speed column (icebreaker)?..");
        }

        point.point_name = wks.cell("D" + std::to_string(row)).value().getString();

        points_info.push_back(std::move(point));
    }

    return points_info;
}

DatesToIceGraph ParseGraphFromJson(
        const std::string& vertices_filepath,
        const std::string& edges_filepath,
        IcebreakersPtr icebreakers) {
    using json = nlohmann::json;
    static const std::array<std::string, 14> ice_dates = {
        "03-Mar-2020",
        "10-Mar-2020",
        "17-Mar-2020",
        "24-Mar-2020",
        "31-Mar-2020",
        "02-Apr-2020",
        "07-Apr-2020",
        "14-Apr-2020",
        "21-Apr-2020",
        "28-Apr-2020",
        "05-May-2020",
        "12-May-2020",
        "19-May-2020",
        "26-May-2020"
    };

    std::ifstream vertices_file(vertices_filepath);
    auto vertices_data = json::parse(vertices_file);
    if (vertices_data.is_null()) {
        throw std::runtime_error(vertices_filepath + " is invalid");
    }

    std::ifstream edges_file(edges_filepath);
    auto edges_data = json::parse(edges_file);
    if (edges_data.is_null()) {
        throw std::runtime_error(edges_filepath + " is invalid");
    }

    DatesToIceGraph date_to_graph;
    Graph graph;

    for (const auto& vertex_json : vertices_data) {
        VertexProperty property;
        property.lat = vertex_json.at("lat").get<float>();
        property.lon = vertex_json.at("lon").get<float>();
        property.name = vertex_json.at("name").get<std::string>();

        boost::add_vertex(property, graph);
    }

    for (const auto& date : ice_dates) {
        date_to_graph[date] = {};
    
        for (auto& ice_graph : date_to_graph[date]) {
            ice_graph = graph;
        }
    }

    for (const auto& edge_json : edges_data) {
        EdgeProperty property;
        property.start_id = edge_json.at("start").get<size_t>();
        property.end_id = edge_json.at("end").get<size_t>();
        property.len = edge_json.at("len").get<float>();

        for (const auto& [date, type_val_json] : edge_json.at("type").items()) {
            property.ice_type = type_val_json.get<int>();

            auto ship_debuffs = GetShipIceDebuff(property.ice_type);
            auto icebreakers_speeds = GetIcebreakerSpeed(property.ice_type, *icebreakers);
            
            size_t graph_type_index = 0;
            for (float debuff : ship_debuffs) {
                if (debuff == 0.0f) {
                    property.weight = std::numeric_limits<float>::infinity();
                } else {
                    property.weight = property.len / debuff;
                }

                boost::add_edge(property.start_id, property.end_id, property, date_to_graph[date][graph_type_index++]);
            }
            for (float icebreaker_speed : icebreakers_speeds) {
                if (icebreaker_speed == 0.0f) {
                    property.weight = std::numeric_limits<float>::infinity();
                } else {
                    property.weight = property.len / icebreaker_speed;
                }

                boost::add_edge(property.start_id, property.end_id, property, date_to_graph[date][graph_type_index++]);
            }
        }
    }

    return date_to_graph;
}

IceGrid ParseIceGrid(const std::string& ice_filepath) {
    OpenXLSX::XLDocument doc{ice_filepath};
    if (!doc.isOpen()) {
        throw std::runtime_error("unable to open " + ice_filepath + " file");
    }

    auto parse_coordinates = [&](std::vector<std::vector<float>>& parse_to, size_t sheet) {
        auto wks = doc.workbook().worksheet(sheet);
        for (size_t i = 1; i <= ICE_MAP_ROWS; ++i) {
            for (size_t j = 1; j <= ICE_MAP_COLUMNS; ++j) {
                auto val_cell = wks.cell(i, j);
                float val;

                if (val_cell.value().type() == OpenXLSX::XLValueType::Integer) {
                    val = static_cast<float>(val_cell.value().get<int>());
                } else if (val_cell.value().type() == OpenXLSX::XLValueType::Float) {
                    val = val_cell.value().get<float>();
                } else {
                    throw std::runtime_error("wtf is the type of (i, j) column (ice_grid)?..");
                }

                parse_to[i - 1][j - 1] = val;
            }
        }
    };

    IceGrid grid;
    
    grid.lon.resize(ICE_MAP_ROWS);
    for (auto& row : grid.lon) {
        row.resize(ICE_MAP_COLUMNS);
    }
    parse_coordinates(grid.lon, 1);

    std::cout << "lon has been read!" << std::endl;

    grid.lat.resize(ICE_MAP_ROWS);
    for (auto& row : grid.lat) {
        row.resize(ICE_MAP_COLUMNS);
    }
    parse_coordinates(grid.lat, 2);

    std::cout << "lat has been read!" << std::endl;

    for (size_t i = 0; i < ICE_WEEKS_AMOUNT; ++i) {
        grid.weekly_ice[i].resize(ICE_MAP_ROWS);
        for (auto& row : grid.weekly_ice[i]) {
            row.resize(ICE_MAP_COLUMNS);
        }
        parse_coordinates(grid.weekly_ice[i], 3 + i);
    
        std::cout << "weekly_ice has been read!" << std::endl;
    }

    return grid;
}

ShipsPtr ParseShipsSchedule(const std::string& dataset_path, const GraphPointsInfo& graph_points_info) {
    OpenXLSX::XLDocument doc{dataset_path};
    if (!doc.isOpen()) {
        throw std::runtime_error("unable to open " + dataset_path + " file");
    }
    
    auto wks = doc.workbook().worksheet(1);

    size_t index = 0;
    auto ships = std::make_shared<Ships>();
    for (size_t row = 2;;++row) {
        if (wks.cell("A" + std::to_string(row)).value().type() == OpenXLSX::XLValueType::Empty) {
            break;
        }

        Ship ship;
        ship.name = wks.cell("A" + std::to_string(row)).value().getString();
        ship.ice_class = FromStringToIceClass(wks.cell("B" + std::to_string(row)).value().getString());

        auto knot_speed_cell = wks.cell("C" + std::to_string(row));
        float knot_speed;
        if (knot_speed_cell.value().type() == OpenXLSX::XLValueType::Integer) {
            knot_speed = static_cast<float>(knot_speed_cell.value().get<int>());
        } else if (knot_speed_cell.value().type() == OpenXLSX::XLValueType::Float) {
            knot_speed = knot_speed_cell.value().get<float>();
        } else {
            throw std::runtime_error("wtf is the type of knot_speed column (ship)?..");
        }

        // узлы -> м/ч
        ship.speed = knot_speed * 1852.0f * 24.0f; // скорость в метрах/день

        ship.cur_pos = getVertID(wks.cell("D" + std::to_string(row)).value().getString(), graph_points_info);
        ship.finish = getVertID(wks.cell("E" + std::to_string(row)).value().getString(), graph_points_info);
        ship.voyage_start_date = wks.cell("F" + std::to_string(row)).value().get<int>();
        ship.id = ShipId{index++};
        
        ships->push_back(std::move(ship));
    }

    return ships;
}

IcebreakersPtr ParseIcebreakers(const std::string& dataset_path, const GraphPointsInfo& graph_points_info) {
    OpenXLSX::XLDocument doc{dataset_path};
    if (!doc.isOpen()) {
        throw std::runtime_error("unable to open " + dataset_path + " file");
    }

    auto icebreakers = std::make_shared<Icebreakers>();
    auto wks = doc.workbook().worksheet(1);
    
    size_t index = 0;

    for (size_t row = 47;;++row) {
        if (wks.cell("C" + std::to_string(row)).value().type() == OpenXLSX::XLValueType::Empty) {
            break;
        }

        Icebreaker icebreaker;
        icebreaker.name = wks.cell("C" + std::to_string(row)).value().getString();
        
        auto knot_speed_cell = wks.cell("D" + std::to_string(row));
        float knot_speed;
        if (knot_speed_cell.value().type() == OpenXLSX::XLValueType::Integer) {
            knot_speed = static_cast<float>(knot_speed_cell.value().get<int>());
        } else if (knot_speed_cell.value().type() == OpenXLSX::XLValueType::Float) {
            knot_speed = knot_speed_cell.value().get<float>();
        } else {
            throw std::runtime_error("wtf is the type of knot_speed column (icebreaker)?..");
        }

        icebreaker.speed = knot_speed * 1852.0f * 24.0f;

        icebreaker.ice_class = FromStringToIceClass(wks.cell("E" + std::to_string(row)).value().getString());
        icebreaker.cur_pos = getVertID(wks.cell("F" + std::to_string(row)).value().getString(), graph_points_info);

        icebreaker.id = IcebreakerId{index++};
        icebreaker.caravan = Caravan{{}, icebreaker.id};

        icebreakers->push_back(std::move(icebreaker));
    }

    return icebreakers;
}

}
