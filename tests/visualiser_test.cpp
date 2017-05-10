#include <iostream>
#include <random>

#include <boost/filesystem.hpp>

#include "io.hpp"
#include "visualiser.hpp"

namespace fs = boost::filesystem;
using PointCloud = cl::PointCloudBase<cl::Point>;

int main(int argc, char **argv)
{
    auto pc = std::make_shared<PointCloud>();

    if (argc == 1) {
        std::mt19937_64 rng;
        std::uniform_real_distribution<float> uni_real(-100.0f, 100.0f);
        for (auto i = 0; i < 100000; ++i) {
            auto r = uni_real(rng);
            auto s = uni_real(rng);
            auto t = uni_real(rng);
            pc->push_back({r, s, t});
        }
    }
    else {
        fs::path p(argv[1]);
        try {
            if (fs::is_directory(p)) {
                fs::directory_iterator endIter;
                for (auto d = fs::directory_iterator(p); d != endIter; ++d) {
                    if (fs::is_regular(d->path())) {
                        cl::io::readFromPCD(d->path().string().c_str(), pc);
                    }
                }
            }
            else {
                cl::io::readFromPCD(p.string().c_str(), pc);
            }
        }
        catch (...) {
            std::cout << "Cannot load points from: " << p << '\n';
            return -1;
        }
    }

    try {
        cl::Visualiser vs{"Visualiser Test"};
        vs.addPointCloud("cloud", pc);
        vs.spin();
    }
    catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
}
