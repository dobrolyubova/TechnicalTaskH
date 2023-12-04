#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <random>
#include <time.h>
#include "polyline.h"
#include "input_parser.h"
#include "tests.h"

std::vector<Point3> read_points(std::string &filename)
{
    std::ifstream in(filename, std::ios::in);

    if (!in.is_open())
    {
        std::cout << "No file: " << filename << "\n";
        return std::vector<Point3>{};
    }

    if (in.eof())
    {
        std::cout << "Empty file\n";
        return std::vector<Point3>{};
    }

    std::stringstream buf;
    buf << in.rdbuf();
    std::vector<Point3> points;
    // by no means exact size! just a lazy workaround
    points.reserve(buf.str().size());
    double x, y, z;

    while (!buf.eof())
    {
       buf >> x; buf >> y; buf >> z;

        points.push_back(Point3{ x, y, z });
    }
    points.shrink_to_fit();
    return points;
}

void output_segments(double& dist, std::vector<size_t>& ids, std::vector<Point3>& projs)
{
   std::cout << "segmments found:\n";
    for (size_t i = 0; i < ids.size(); ++i)
    {
        std::cout << ids[i] << ", projection (" << projs[i].x << ", " << projs[i].y << ", " << projs[i].z << "),\n";

    }
    std::cout << "with minimal distance d = " << dist;
    std::cout << "\n";
}

///____________________________________________________________________________________

int main(int argc, char** argv)
{
    std::filesystem::path cwd = std::filesystem::current_path();
    std::cout << cwd.string();
    std::string filename = "";
    Point3 P;
    if (argc < 5)
    {
        std::cout << "\nInvalid input! Not enough arguments\n";
        return EXIT_FAILURE;
    }
    filename = std::string(argv[1]);
    P.x = atof(argv[2]);
    P.y = atof(argv[3]);
    P.z = atof(argv[4]);
    
    std::vector<Point3> points = read_points(filename);
    Polyline* p = nullptr;
    try { p = new Polyline(points); }
    catch (std::runtime_error& e)
    {
        std::cout << e.what() << "\n Invalid input!\n";
        return EXIT_FAILURE;
    }
    auto [dist, ids, projs] = p->locate_point(P);
    output_segments(dist, ids, projs);
    delete p;
    return 1;
   
}

