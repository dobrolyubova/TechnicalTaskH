#pragma once
#include "polyline.h"

std::vector<Point3> read_points(std::string& filename);
void output_segments(double& dist, std::vector<size_t>& ids, std::vector<Point3>& projs);

namespace tests
{
	void test_example1();
	void test_example2();
	void test_example3();
	void test_point_is_segment_node();
	void test_point_outside_bbox();
	void test_projection_in_segment(Polyline* p, Point3& P);

}
