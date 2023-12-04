#include <fstream>
#include <iostream>
#include <time.h>
#include "polyline.h"

// you could say, it is a magical constant...
// close to 8^3 (so, at least 3 levels in octree), and also the fact that
// greedy search in 5000 elements takes fairly negligible time
static const size_t MAX_OCTANT = 5000;
static const double eps_pis = 1.e-5;

std::tuple<double, Point3> Segment::euc_dist(const Point3& p) const
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

bool Segment::contains_point(const Point3& p) const
{
	Vec3 s_tangent = *(this->p1) - *(this->p2);
	Vec3 s_beg_proj = (p - *(this->p1));

	double S = (s_tangent.cross(s_beg_proj)).norm() * 0.5;
	
	if (S > eps_pis)
		return false;
	return true;
}

Polyline::Polyline(std::vector<Point3>& v)
{
	if (v.size() < 2)
		std:throw std::runtime_error("Polyline implies at least two points!");
	points.resize(v.size());
	std::move(v.begin(), v.end(), points.begin());

	// xmin, xmax, ymin, ymax, zmin, zmax
	std::array<double, 6> bounds = { 
		std::numeric_limits<double>::max(),
	-std::numeric_limits<double>::max(),
	std::numeric_limits<double>::max(),
	-std::numeric_limits<double>::max(),
	std::numeric_limits<double>::max(),
	- std::numeric_limits<double>::max() 
	};

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
				Point3{bounds[1], bounds[3], bounds[5]
	}
	};

	octree = std::make_shared<Octree<Segment>>(MAX_OCTANT, true);
	octree->construct(this->bounds,	points);
}

std::tuple<double, std::vector<size_t>, std::vector<Point3>> Polyline::locate_point(const Point3& p)
{
	return octree->locate_point(p);
}

std::optional<Segment> Polyline::get_segment(const size_t &id)
{
	if(id < points.size() - 1)
		return Segment{&points[id], &points[id+1]};
	return {};
}


