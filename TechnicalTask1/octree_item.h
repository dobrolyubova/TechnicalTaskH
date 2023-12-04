#pragma once
#ifndef OCTREE_ITEM_H
#define OCTREE_ITEM_H
#include <vector>
#include <memory>
#include <tuple>
#include "geo_units.h"

using namespace geo_units;

// Axis-aligned bounding box
struct AABBox
{
	Point3 lMin, rMax;

	template <class T>
	bool is_inside(const T& s) const;
	bool is_inside(const Point3& s) const;
	std::array<Point3, 8> get_all_points() const;
	std::array<Point3, 4> get_plane_points(const size_t &id) const;
};

template <class T>
class TreeItem
{
public:
	std::vector<std::shared_ptr<TreeItem>> descendants;
	std::vector<std::shared_ptr<T>> data;
	AABBox bounds;

	TreeItem(AABBox bounds) : bounds(bounds) {}

	~TreeItem()
	{
		descendants.clear();
		std::vector<std::shared_ptr<TreeItem>>().swap(descendants);
	}

	void split();
	std::tuple<bool, size_t> is_inside(T& obj);
};

#endif


