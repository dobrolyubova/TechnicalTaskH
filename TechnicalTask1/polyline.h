#pragma once
#ifndef POLYLINE_H
#define POLYLINE_H
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <array>
#include "octree.h"

using namespace geo_units;

struct Segment
{
	Point3 *p1, *p2;
	size_t id;
	std::tuple<double, Point3> euc_dist(const Point3& p) const;
	bool contains_point(const Point3& p) const;
};

class Polyline
{
public:
	Polyline(std::vector<Point3>&v);
	// Searches for the nearest segment to a point p 
	// A distance between point P and degment is defined as
	// a length of a projection of a point P onto the segment (in the plane fromed of two segment points and point P)
	// or a distance to the nearest segment vertex, if a projections of P falls out of segment
	// returns: 
	//		mininmum distance, 
	//		ids of the closest segments, 
	//		projections onto closest segments
	std::tuple<double, std::vector<size_t>, std::vector<Point3>> locate_point(const Point3& p);
	std::optional<Segment> get_segment(const size_t &id);
	double get_max_span() const
	{
		auto diff = bounds.rMax - bounds.lMin;
		return std::max(std::max(diff.x , diff.y), diff.z);
	}
private:
	std::vector<Point3> points;
	AABBox bounds;
	// i-th segment: {points[i], points[i+1]}
	std::shared_ptr<Octree<Segment>> octree;
};

#endif