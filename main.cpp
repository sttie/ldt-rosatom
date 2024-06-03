#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

#include "src/parser.h"

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif
    
    auto ships = parser::ParseShipsSchedule("C:\\Users\\sarutiunian\\Desktop\\work\\ldt-rosatom\\dataset\\Расписание движения судов.xlsx");
    for (auto& ship : ships) {
        std::cout << ship.name << ": ice_class=" << static_cast<int>(ship.ice_class)
                  << ", knot_speed=" << ship.knot_speed
                  << ", departure=" << ship.departure
                  << ", destination=" << ship.destination
                  << ", voyage_start_date=" << ship.voyage_start_date
                  << "\n" << std::endl;
    }

    auto icebreakers = parser::ParseIcebreakers("C:\\Users\\sarutiunian\\Desktop\\work\\ldt-rosatom\\dataset\\Расписание движения судов.xlsx");
    for (auto& icebreaker : icebreakers) {
        std::cout << icebreaker.name << ": "
                  << "ice_class=" << static_cast<int>(icebreaker.ice_class)
                  << ", knot_speed=" << icebreaker.knot_speed
                  << ", departure=" << icebreaker.departure
                  << "\n" << std::endl;
    }

    return 0;
}