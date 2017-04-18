//
// Created by janko on 11.4.2017.
//

#include <iostream>

#include "cl.hpp"
#include "visualiser.h"
#include <random>

using namespace cl;

int main(int argc, char** argv){
    auto pc = std::make_shared<PointCloud>();
    auto denom = 0.0000000001f;
    for(auto i = 0; i < 100000; ++i){
        auto r = static_cast<float>(std::rand()) * denom;
        auto s = static_cast<float>(std::rand()) * denom;
        auto t = static_cast<float>(std::rand()) * denom;
        pc->push_back({r, s, t});
    }

    Visualiser vs{"Motion Test"};
    vs.addPointCloud("cloud1", pc);
    vs.spin();
}