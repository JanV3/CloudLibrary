#ifndef CL_IO_HPP
#define CL_IO_HPP

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

        inline std::ostream &operator<<(std::ostream &input, PCDHeader &header)
        {
            input << "Version: " << header.version << std::endl;
            input << "Width: " << header.width << std::endl;
            input << "Height: " << header.height << std::endl;
            input << "Points: " << header.points << std::endl;
            input << "Data: " << header.data << std::endl;
            return input;
        };

        inline void readFromPCD(std::string path, PointCloud::Ptr cloud)
        {
            std::ifstream file(path);
            if (!file.is_open())
                return;

            std::string line;
            int i = 0;
            bool header_ok = false;
            PCDHeader header;
            while (std::getline(file, line)) {
                if (line[0] == '#')
                    continue;

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

		inline void saveToFile(std::string path, PointCloud& cloud)
		{
			std::ofstream f(path);
			f << cloud.size() << '\n';
			for (auto p = cloud.begin(); p != cloud.end(); ++p) {
				f << p->x << " " << p->y << " " << p->z << '\n';
			}
		}

		inline void loadFromFile(std::string path, PointCloud& cloud)
		{
			std::ifstream f(path);
			size_t points;
			f >> points;
			std::string line;
			while (std::getline(f, line)) {
				std::istringstream iss(line);
				float x, y, z;
				iss >> x >> y >> z;
				cloud.push_back({ x, y, z });
			}
		}

		inline void saveToBin(std::string path, std::vector<PointCloud::Ptr> clouds)
		{
			std::ofstream f(path, std::ios::binary);

			// write number of clouds
			auto cloudsNumber = static_cast<unsigned int>(clouds.size());
			f.write(reinterpret_cast<char*>(&cloudsNumber), sizeof(cloudsNumber));

			for (const auto& c : clouds) {
				// write size of cloud
				auto size = static_cast<unsigned int>(c->size());
				f.write(reinterpret_cast<char*>(&size), sizeof(size));

				auto name = c->getName();

				// check if cloud have name and set flag
				unsigned char flags = 0;
				if (!name.empty()) {
					flags = 0x10;
				}
				f.write(reinterpret_cast<char*>(&flags), sizeof(flags));

				// write cloud name if exists
				if (flags == 0x10) {
					f.write(name.c_str(), name.size() + 1);
				}

				// write cloud data
				f.write(reinterpret_cast<const char*>(c->data()), sizeof(float) * size * 3);
			}
		}

		inline void loadFromBin(std::string path, std::vector<PointCloud::Ptr>& clouds)
		{
			std::ifstream f(path, std::ios::binary);

			// read clouds number
			unsigned int cloudsNumber;
			f.read(reinterpret_cast<char*>(&cloudsNumber), sizeof(cloudsNumber));
			
			for (unsigned int i = 0; i < cloudsNumber; ++i) {
				// create cloud
				auto cloud = std::make_shared<PointCloud>();

				// read size of cloud
				unsigned int size;
				f.read(reinterpret_cast<char*>(&size), sizeof(size));

				// read flags for cloud
				unsigned char flags;
				f.read(reinterpret_cast<char*>(&flags), sizeof(flags));

				// if cloud have name, read it
				if (flags == 0x10) {
					std::string name;
					auto beginPos = f.tellg();
					char c;
					do {
						f.read(&c, 1);
					} while (c != '\0');
					auto endPos = f.tellg();
					auto nameLength = endPos - beginPos - 1;
					name.resize(nameLength);
					f.seekg(beginPos);
					f.read(&name[0], nameLength);
					cloud->setName(name);
					f.seekg(endPos);
				}

				// read all data points
				cloud->resize(size);
				f.read(reinterpret_cast<char*>(cloud->data()), sizeof(float) * size * 3);

				// store cloud to vector
				clouds.push_back(cloud);
			}
		}
    } // namespace io
} // namespace cl

#endif /* CL_IO_HPP */
