//
// Created by janko on 11.4.2017.
//

#include <iostream>

#include "cl.hpp"
#include "io.h"
#include "visualiser.h"
#include <random>
#include <boost/filesystem.hpp>

namespace fs=boost::filesystem;

using namespace cl;

int main(int argc, char** argv){
    if(argc != 2){
        std::cout << "Program takes one argument: path to directory or pcd file" << std::endl;
        return -1;
    }

    auto pc = std::make_shared<PointCloud>();
    auto denom = 0.0000000001f;
    for(auto i = 0; i < 100000; ++i){
        auto r = static_cast<float>(std::rand()) * denom;
        auto s = static_cast<float>(std::rand()) * denom;
        auto t = static_cast<float>(std::rand()) * denom;
        pc->push_back({r, s, t});
    }

    auto pc2 = std::make_shared<PointCloud>();

    fs::path p(argv[1]);

    if(fs::is_directory(p)){
        fs::directory_iterator endIter;
        for(auto d = fs::directory_iterator(p); d != endIter; ++d){
            if(fs::is_regular(d->path())){
                io::readFromPCD(d->path().string().c_str(), pc2);
            }
        }
    } else {
        io::readFromPCD(p.string().c_str(), pc2);
    }

    Visualiser vs{"Motion Test"};
    vs.addPointCloud("cloud1", pc2);
    vs.spin();
}
