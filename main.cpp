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
    //if(argc != 2){
        //std::cout << "Program takes one argument: path to directory or pcd file" << std::endl;
        //return -1;
    //}
    //

    std::mt19937_64 rng;
    std::uniform_real_distribution<float> uni_real(-10.0f, 10.0f);

    auto pc = std::make_shared<PointCloud>();
    for(auto i = 0; i < 1000; ++i){
        auto r = uni_real(rng);
        auto s = uni_real(rng);
        auto t = uni_real(rng);
        pc->push_back({r, s, t});
    }

    std::cout << *pc << std::endl;

    //auto pc2 = std::make_shared<PointCloud>();

    //fs::path p(argv[1]);

    //if(fs::is_directory(p)){
        //fs::directory_iterator endIter;
        //for(auto d = fs::directory_iterator(p); d != endIter; ++d){
            //if(fs::is_regular(d->path())){
                //io::readFromPCD(d->path().string().c_str(), pc2);
            //}
        //}
    //} else {
        //io::readFromPCD(p.string().c_str(), pc2);
    //}

    Visualiser vs{"Motion Test"};
    vs.addPointCloud("cloud1", pc);
    vs.spin();
}
