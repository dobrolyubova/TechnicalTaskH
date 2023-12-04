#include <iostream>
#include <filesystem>
#include "tests.h"

/// 
/// This is obviously NOT how test are supposed to look like, I just have no experience with testing
/// 

namespace tests
{
    void test_example1()
    {
        std::filesystem::path cwd = std::filesystem::current_path();
        std::string filename = cwd.string() + "/tests/example1.txt";
        std::vector<Point3> points = read_points(filename);
        Polyline p(points);
        Point3 P{ 2, 0.5, 0.5 };
        auto [dist, ids, projs] = p.locate_point(P);

        if (ids.size() != 2)
            throw std::runtime_error("Incorrect number of closest segments!");
        if (!(projs[0] == Point3{ 1.75, 0.75, 0 }))
            throw std::runtime_error("Incorrect projection point!");
        if (!(projs[1] == Point3{ 2.25, 1, 0.25 }))
            throw std::runtime_error("Incorrect projection point!");

        output_segments(dist, ids, projs);
    }
    void test_example2()
    {
        std::filesystem::path cwd = std::filesystem::current_path();
        std::string filename = cwd.string() + "/tests/example2.txt";
        std::vector<Point3> points = read_points(filename);
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
        std::filesystem::path cwd = std::filesystem::current_path();
        std::string filename = cwd.string() + "/tests/example3.txt";
        std::vector<Point3> points = read_points(filename);
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
        std::filesystem::path cwd = std::filesystem::current_path();
        std::string filename = cwd.string() + "/tests/small_teststxt";
        std::vector<Point3> points = read_points(filename);

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
        std::filesystem::path cwd = std::filesystem::current_path();
        std::string filename = cwd.string() + "/tests/small_teststxt";
        std::vector<Point3> points;
        try {
            points = read_points(filename);
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

        return 1;
    }

}
