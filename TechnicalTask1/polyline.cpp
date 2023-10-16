#include <fstream>
#include <iostream>
#include <time.h>
#include "polyline.h"

// you could say, it is a magical constant...
// close to 8^3 (so, at least 3 levels in octree), and also the fact that
// greedy search in 5000 elements takes fairly negligible time
static const size_t MAX_OCTANT = 5000;

std::tuple<double, Point3> Segment::euc_dist(Point3& p) const
{
	// if the Segment end == Segment start:
	if (*p1 == *p2)
		return std::make_tuple(p1->euc_dist(p), *p1);

	double inv_norm = (1. / p2->euc_dist(*p1));
	// tangent vector for the segment
	Vec3 t = (*p2 - *p1) * inv_norm;
	Vec3 p1p = p - *p1;
	Vec3 p2p = p - *p2;

	if (p1p.dot(t) < 0)
		return std::make_tuple(p1->euc_dist(p), *p1);
	if(p2p.dot(t) > 0)
		return std::make_tuple(p2->euc_dist(p), *p2);

	auto proj_dist = p1p.dot(t);
	Point3 p_proj{ p1->x + proj_dist * t.v[0], p1->y + proj_dist * t.v[1], p1->z + proj_dist * t.v[2] };

	auto n = (p1p.cross(t));
	double dist = n.norm();

	return std::make_tuple(dist, p_proj);
}

bool Segment::contains_point(Point3& p)
{
	return false;
}


Polyline::Polyline(std::vector<Point3>& v)
{
	if (v.size() < 2)
		std:throw std::runtime_error("Polyline implies at least two points!");
	points.resize(v.size());
	std::move(v.begin(), v.end(), points.begin());

	// xmin, xmax, ymin, ymax, zmin, zmax
	std::array<double, 6> bounds;
	bounds[0] = bounds[2] = bounds[4] = std::numeric_limits<double>::max();
	bounds[1] = bounds[3] = bounds[5] = -std::numeric_limits<double>::max();

	for (auto& p : points)
	{
		if (p.x < bounds[0])
			bounds[0] = p.x;
		if (p.x > bounds[1])
			bounds[1] = p.x;
		if (p.y < bounds[2])
			bounds[2] = p.y;
		if (p.y > bounds[3])
			bounds[3] = p.y;
		if (p.z < bounds[4])
			bounds[4] = p.z;
		if (p.z > bounds[5])
			bounds[5] = p.z;
	}
	this->bounds = AABBox{
				Point3{bounds[0], bounds[2], bounds[4]},
				Point3{bounds[1], bounds[3], bounds[5]}
	};

	octree = std::make_shared<Octree<Segment>>(MAX_OCTANT, true);
	octree->construct(this->bounds,	points);
}

std::tuple<double, std::vector<size_t>, std::vector<Point3>> Polyline::locate_point(Point3& p)
{
	return octree->locate_point(p);
}

std::tuple<double, std::vector<size_t>, std::vector<Point3>> Polyline::locate_point_greedy(Point3& p)
{
	size_t sz_seg = points.size() - 1;
	double min_dist = std::numeric_limits<double>::max();
	std::vector<size_t> closest_seg_ids;
	closest_seg_ids.reserve(sz_seg);
	std::vector<Point3> projection_points;
	projection_points.reserve(sz_seg);

	for (size_t i_seg = 0; i_seg < sz_seg; ++i_seg)
	{
		auto [dist, proj] = get_segment(i_seg)->euc_dist(p);
		if (is_equal(min_dist, dist))
		{
			closest_seg_ids.push_back(i_seg);
			projection_points.push_back(proj);
		}
		if (min_dist > dist)
		{
			min_dist = dist;
			projection_points = std::vector<Point3>{ proj };
			closest_seg_ids = std::vector<size_t>{ i_seg };
		}
		
	}
	return std::make_tuple(min_dist, closest_seg_ids, projection_points);
}

std::optional<Segment> Polyline::get_segment(size_t id)
{
	if(id < points.size() - 1)
		return Segment{&points[id], &points[id+1]};
	return {};
}


