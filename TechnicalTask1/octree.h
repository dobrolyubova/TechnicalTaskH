#pragma once
#include <list>
#include "octree_item.h"

template <class T>
class Octree
{
	std::shared_ptr<TreeItem<T>> root;
	size_t MAX_R;	// max data items in box
	bool verbose = false;

	void push_back(std::shared_ptr<T> s, std::shared_ptr<TreeItem<T>>& cur_tree);
	
	std::tuple<
		double, 
		std::vector<size_t>, 
		std::vector<Point3>
	> locate_point(const Point3& p, std::shared_ptr<TreeItem<T>>& tree);

	// Projects the point p, which is outside the BBox, onto the closest BBox plane
	// p - point to be projected;
	// tree - Octree node whose BBox does not contain the point p 
	// returns the coordinates of projection
	Point3 project_closest(const Point3& p, std::shared_ptr<TreeItem<T>>& tree);
	
	// Recursivey searches octree for point p_proj (whinch might be equal to p, 
	// if p is inside octree root BBox, or is the projection onto the closest Bbox plane otherwise)
	// returns:
	//		mininmum distance, 
	//		ids of the closest segments, 
	//		projections onto closest segments
	// this, actually, is a violation of obj-oriented principles, basically, 
	// octree should not care for the distances and projections, and should only return the octant
	// to then use the greedy search on it
	std::tuple<
		double, 
		std::vector<size_t>, 
		std::vector<Point3>>
		depth_first_search(const Point3& p, Point3& p_proj, std::shared_ptr<TreeItem<T>>& tree);

public:
	
	Octree(size_t maxR, bool verbose = false) : MAX_R(maxR), verbose(verbose) {};
	~Octree(){}
	std::shared_ptr<TreeItem<T>> construct(AABBox &bounds, std::vector<Point3>&);
   	void insert(std::shared_ptr<T> s, std::shared_ptr<TreeItem<T>>& cur_tree);
   	std::tuple<double, std::vector<size_t>, std::vector<Point3>> locate_point(const Point3& p);

 };