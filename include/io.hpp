#ifndef IO_H
#define IO_H

#include <fstream>
#include <sstream>

#include "point_cloud.hpp"

namespace cl {
namespace io {
struct PCDHeader {
    std::string version;
    std::vector<std::string> fields;
    std::vector<int> size;
    std::vector<std::string> type;
    std::vector<int> count;
    unsigned int width;
    unsigned int height;
    std::vector<float> viewpoint;
    unsigned int points;
    std::string data;
};

std::ostream &operator<<(std::ostream &input, PCDHeader &header)
{
    input << "Version: " << header.version << std::endl;
    input << "Width: " << header.width << std::endl;
    input << "Height: " << header.height << std::endl;
    input << "Points: " << header.points << std::endl;
    input << "Data: " << header.data << std::endl;
    return input;
};

void readFromPCD(std::string path, PointCloud::Ptr cloud)
{
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    int i = 0;
    bool header_ok = false;
    PCDHeader header;
    while (std::getline(file, line)) {
        if (line[0] == '#') continue;

        if (header_ok) {
            std::istringstream iss(line);
            float x, y, z;
            iss >> x >> y >> z;
            cloud->push_back({x, y, z});
            continue;
        }

        std::istringstream iss(line);

        std::string name;
        iss >> name;

        if (name == "VERSION") {
            iss >> header.version;
        }

        if (name == "FIELDS") {
            std::string f;
            while (iss >> f)
                header.fields.push_back(f);
        }

        if (name == "SIZE") {
            int s;
            while (iss >> s)
                header.size.push_back(s);
        }

        if (name == "TYPE") {
            std::string t;
            while (iss >> t)
                header.type.push_back(t);
        }

        if (name == "COUNT") {
            int c;
            while (iss >> c)
                header.count.push_back(c);
        }

        if (name == "WIDTH") {
            iss >> header.width;
        }

        if (name == "HEIGHT") {
            iss >> header.height;
        }

        if (name == "VIEWPOINT") {
            float v;
            while (iss >> v)
                header.viewpoint.push_back(v);
        }

        if (name == "POINTS") {
            iss >> header.points;
        }

        if (name == "DATA") {
            iss >> header.data;
            header_ok = true;
        }

        if (header_ok && header.data == "binary") {
            for (unsigned int p = 0; p < header.points; ++p) {
                float x, y, z;
                file.read(reinterpret_cast<char *>(&x), header.size[0]);
                file.read(reinterpret_cast<char *>(&y), header.size[1]);
                file.read(reinterpret_cast<char *>(&z), header.size[2]);
                cloud->push_back({x, y, z});
            }
            break;
        }
    }

    return;
}
} // namespace io
} // namespace cl

#endif /* IO_H */
