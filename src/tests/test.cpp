#include "test.h"

#include "path.h"
#include "algos/algos.h"

#include <iostream>

namespace tests {

namespace {

void TestTrivial() {
    std::cout << "TestTrivial()..." << std::endl;

    Graph graph;
}

}

void TestAll() {
    TestTrivial();
}

}