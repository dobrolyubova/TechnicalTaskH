#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>
#include <sstream>
#include <random>
#include <time.h>
#include "polyline.h"
#include "input_parser.h"

// Generates a list of N points with coordinates between lb and up
// used to generate test files
void generate_points(size_t N_points, double lb, double ub, std::string out_name)
{
    std::uniform_real_distribution<double> unif(lb, ub);
    std::default_random_engine re;
    re.seed(std::chrono::system_clock::now().time_since_epoch().count());

    std::ofstream out(out_name, std::ios::out);

    size_t curr_p = 0;
    while (curr_p < N_points)
    {
        double val = unif(re);
        out << val << " ";
        val = unif(re);
        out << val << " ";
         val = unif(re);
        out << val << "\n";

        ++curr_p;
    }
    out.close();
}

std::vector<Point3> read_points(std::string filename)
{
    std::ifstream in(filename, std::ios::in);

    if (!in.is_open())
    {
        std::cout << "No such file\n";
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

void clean_run(Polyline* p, Point3& P)
{
    auto [dist, ids, projs] = p->locate_point(P);
    output_segments(dist, ids, projs);
}

int user_cycle(std::function< void(Polyline* p, Point3& P) > foo, std::string msg = {})
{
    std::string filename;
    std::cout << "Enter name of .txt file for polyline:\n";
    std::cin >> filename;
    std::cout << "Initializing polyline...\n";

    std::vector<Point3> points = read_points(filename);
    Polyline* p = nullptr;
    try { p = new Polyline(points); }
    catch (std::runtime_error& e)
    {
        std::cout << e.what() << "\n Invalid input!\n";
        return EXIT_FAILURE;
    }
    std::cout << "Initialization done\n";
    std::string s;


    while (s != "n")
    {
        std::cout << "Enter point coordinates\n";
        double x, y, z;
        std::cin >> x >> y >> z;
        Point3 P{ x , y, z };
        std::cout << "Point P = {" << x << ", " << y << ", " << z << "}\n";

        try {
            foo(p, P);
        }
        catch (std::runtime_error& e) {
            std::cout << e.what() << msg << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "Continue? (y/n)\n";
        std::cin >> s;
    }
    delete p;
    return EXIT_SUCCESS;
}

/// 
/// This is obviously NOT how test are supposed to look like, I just have no experience with testing
/// 

namespace tests
{
    void test_example1()
    {
        std::vector<Point3> points = read_points("example1.txt");
        Polyline p(points);
        Point3 P{ 2, 0.5, 0.5 };
        auto [dist, ids, projs] = p.locate_point(P);
        
        if(ids.size() != 2)
            throw std::runtime_error("Incorrect number of closest segments!");
        if (!(projs[0] == Point3{ 1.75, 0.75, 0 }))
            throw std::runtime_error("Incorrect projection point!");
        if (!(projs[1] == Point3{ 2.25, 1, 0.25 }))
            throw std::runtime_error("Incorrect projection point!");

        output_segments(dist, ids, projs);
    }
    void test_example2()
    {
        std::vector<Point3> points = read_points("example2.txt");
        Polyline p(points);
        
        Point3 P{ 1, 1, 1 };
        auto [dist, ids, projs] = p.locate_point(P);

        if (ids.size() != 4)
            throw std::runtime_error("Incorrect number of closest segments!");
        if (!(projs[0] == Point3{ 1, 0, 0 }))
            throw std::runtime_error("Incorrect projection point!");
        if (!(projs[1] == Point3{ 2, 1, 0 }))
            throw std::runtime_error("Incorrect projection point!");
        if (!(projs[2] == Point3{ 1, 2, 0 }))
            throw std::runtime_error("Incorrect projection point!");
        if (!(projs[3] == Point3{ 0, 1, 0 }))
            throw std::runtime_error("Incorrect projection point!");

        output_segments(dist, ids, projs);
    }


    void test_example3()
    {
        std::vector<Point3> points = read_points("example3.txt");
        Polyline p(points);
        Point3 P{ 3, 3, 3 };
        auto [dist, ids, projs] = p.locate_point(P);

        output_segments(dist, ids, projs);

        if (ids.size() != 2)
            throw std::runtime_error("Incorrect number of closest segments!");
        if (!(projs[0] == Point3{ 2, 2, 0 }))
            throw std::runtime_error("Incorrect projection point!");
        if (!(projs[1] == projs[0]))
            throw std::runtime_error("Incorrect projection point!");

    }
    
    // t 4
    void test_point_is_segment_node()
    {
        std::vector<Point3> points = read_points("small_tests.txt");
        Polyline p(points);
        size_t indx = rand() % (points.size() - 1) + 1;
        Point3 P = points[indx];
        auto [dist, ids, projs] = p.locate_point(P);

        output_segments(dist, ids, projs);

        // Should have returned exactely two segments
        if (ids.size() != 2)
            throw std::runtime_error("Lost some segments!");

        auto seg = p.get_segment(ids[0]);
        if (!((*(seg->p1)) == points[indx] || (*(seg->p2)) == points[indx]))
            throw std::runtime_error("Test point is segment node failed!");


    }
    // t 5
    void test_point_outside_bbox()
    {
        std::vector<Point3> points;
        try {
            points = read_points("small_tests.txt");
        }
        catch (std::runtime_error& e) {
            std::cout << e.what() << "\n";
            throw;
        }

        Polyline p(points);

        Point3 P{ -1000., 0.023, -2.35 };
        auto [dist, ids, projs] = p.locate_point(P);

        double min2 = std::numeric_limits<double>::max();
        for (auto& p_ln : points)
        {
            int stop = 1;
            double dist2 = P.euc_dist(p_ln);
            if (min2 > dist2)
                min2 = dist;
        }

        output_segments(dist, ids, projs);
        if (!(projs[0] == Point3{ 0.0, 2.0, 4.0 }))
            throw std::runtime_error("Test point is segment node failed!");

    }
    // t 6
    void test_against_greedy(Polyline* p, Point3& P)
    {
        time_t start = clock();
        auto ret_oct = p->locate_point(P);
        auto ticks1 = (clock() - start);
        auto finish1 = ticks1 / double(CLOCKS_PER_SEC);

        start = clock();
        auto ret_greedy = p->locate_point_greedy(P);
        auto ticks2 = (clock() - start);
        auto finish2 = ticks2 / double(CLOCKS_PER_SEC);

        std::cout << "Octree: " << finish1 << "\nGreedy: " << finish2 << "\n\n";

        // This is not how one's supposed to make tests, though
        if (!(std::get<0>(ret_oct) == std::get<0>(ret_greedy)))
            throw std::runtime_error("Minimal distance obtained via octree equal to minimal distance by greedy search");
    }
    // t 7
    // tests if the returned P projection actually belongs to segment
    void test_projection_in_segment(Polyline* p, Point3& P)
    {
        constexpr double eps = std::numeric_limits<float>::epsilon();
        auto [dist, ids, projs] = p->locate_point(P);
        // if the area of the triangle formed by segment vertices and projection point is
        // sufficiently small, then we consider the projection point to belong to the segment
        // although, the problem of what should be considered sufficiently small is not thoroughly adressed here
        for (size_t i = 0; i < ids.size(); ++i)
        {
            auto seg = p->get_segment(ids[i]);
            Vec3 s_tangent = *(seg->p1) - *(seg->p2);
            Vec3 s_beg_proj = (projs[i] - *(seg->p1));

            double S = (s_tangent.cross(s_beg_proj)).norm() * 0.5;
            std::cout << S << "\n";
            if (S > eps)
                throw std::runtime_error("Projected point not in closest segment!");
        }
    }

    int run_tests()
    {
        int ret = EXIT_SUCCESS;

        try {
            tests::test_example1();
        }
        catch (std::runtime_error& e) {
            std::cout << e.what() << "Example1 test failed!" << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Example1 test passed!" << "\n\n";
        
        try {
            tests::test_example2();
        }
        catch (std::runtime_error& e) {
            std::cout << e.what() << "Example2 test failed!" << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Example2 test passed!" << "\n\n";

        try {
            tests::test_example3();
        }
        catch (std::runtime_error& e) {
            std::cout << e.what() << "Example3 test failed!" << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Example3 test passed!" << "\n\n";

        try {
            tests::test_point_is_segment_node();
        }
        catch (std::runtime_error& e) {
            std::cout << e.what() << "Point is segment node test failed!" << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Point is segment node test passed!" << "\n\n";

        try {
            tests::test_point_outside_bbox();
        }
        catch (std::runtime_error& e) {
            std::cout << e.what() << "Point outside polyline boundary box test failed!" << "\n";
            return EXIT_FAILURE;
        }
        std::cout << "Point outside polyline boundary box test passed!" << "\n\n";

        // not unit tests, probably, should not be here
        ret = user_cycle(tests::test_projection_in_segment,
            "PointInSegment test failed!");
        
        ret = user_cycle(tests::test_against_greedy,
            "Test against greedy failed!");
       
        return ret;
    }

}
///____________________________________________________________________________________


int main(int argc, char** argv)
{
    int test = -1;
    InputParser input(argc, argv);
    // option t to run tests
    if (input.cmdOptionExists("t"))
    {
        test = std::atoi(input.getCmdOption("t").c_str());
        std::cout << "Runnig tests \n\n";
        tests::run_tests();
    }
    else
    {
        // option g to generate test file
        if (input.cmdOptionExists("g"))
        {
            size_t N;
            double min = 0, max = 0;
            std::cout << "Enter number of points generated polyline:\n";
            std::cin >> N;
            std::cout << "Enter lower and upper bounds for point coordinates:\n";
            std::cin >> min;
            std::cin >> max;
            std::string filename;
            std::cout << "Enter name of .txt file for generated polyline:\n";
            std::cin >> filename;
            std::cout << "Generating polyline...\n";
            generate_points(N, min, max, filename);
            return EXIT_SUCCESS;
        }
        return user_cycle(clean_run);
    }
   
}

