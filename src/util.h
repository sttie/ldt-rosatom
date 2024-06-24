#pragma once

#include <chrono>
#include <iostream>

struct TimerScope {
    TimerScope(std::string name) 
        : name(std::move(name))
        , m_start { std::chrono::steady_clock::now() }
    {}

    ~TimerScope() {
        using namespace std::chrono;
        auto life = steady_clock::now() - m_start;
        std::cout << "[" << name << "]" << " ms: " << duration_cast<milliseconds>(life).count() << std::endl;
    }

private:
    std::string name;
    std::chrono::steady_clock::time_point m_start;
};
