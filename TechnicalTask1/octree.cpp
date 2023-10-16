#include <algorithm>
#include <iostream>
#include <time.h>
#include "octree.h"
#include "polyline.h"

std::shared_ptr<TreeItem<Segment>> Octree<Segment>::construct(AABBox bounds, std::vector<Point3>& points)
{
	time_t start = clock();
	
	root = std::make_shared<TreeItem<Segment>>(bounds);
	root->split();

	if(verbose)
		std::cout << "octree: constructing...\n";
	
	size_t sz = points.size() - 1;
	for (size_t i = 0; i < sz; ++i)
		insert(std::make_shared<Segment>(Segment{&points[i], &points[i+1], i }), root);
	
	auto ticks1 = (clock() - start);
	if (verbose)
		std::cout << "Octree constructinon : " << ticks1 / double(CLOCKS_PER_SEC) << "\n";

	return root;
}

template <class T>
void Octree<T>::push_back(std::shared_ptr<T> s, std::shared_ptr<TreeItem<T>>& cur_tree)
{
	if (cur_tree->data.capacity() == 0)
		cur_tree->data.reserve(MAX_R);

	if (cur_tree->data.size() <= MAX_R || cur_tree->descendants.size())
		cur_tree->data.push_back(s);
	else
	{
		cur_tree->split();

		std::vector < std::shared_ptr <T>> elements = {};
		elements.swap(cur_tree->data);

		for (auto& e : elements)
		{
			insert(e, cur_tree);
		}

	}
}


template <class T>
void Octree<T>::insert(std::shared_ptr<T> s, std::shared_ptr<TreeItem<T>>& tree)
{
	size_t CTI_Dsize = tree->descendants.size();
	if(CTI_Dsize == 0)
		push_back(s, tree);

	for (int k = 0; k < CTI_Dsize; ++k)
	{
		auto [inside, both_inside] = tree->descendants[k]->is_inside(*s);
		// if both points of the segment are inside one child box,
		// then the segment belongs to this box
		if (both_inside)
			insert(s, tree->descendants[k]);
		else if (inside)
		{
			// if each point of the segment belongs to different child box, 
			// the segment is placed into parent box
			push_back(s, tree);
			break;

		}
	}
}

std::tuple<double, std::vector<size_t>, std::vector<Point3>> Octree<Segment>::locate_point(Point3& p)
{
	return locate_point(p, root);
}

template <>
std::tuple<double, std::vector<size_t>, std::vector<Point3>> Octree<Segment>::locate_point(Point3& p, std::shared_ptr<TreeItem<Segment>>& tree)
{
	// if the point is outside the Octree root BBox, obtain the closest projection-point
	// if the projection is outside BBox, take the closest BBox point
	if (tree != nullptr)
	{
		Point3 p_proj;
		if (!root->bounds.is_inside(p))
		{
			p_proj = project_closest(p, tree);
		}
		else
		{
			p_proj = p;
		}
		// now, we'll check if the p_proj is inside the subboxes,
		// but count distance to p anyway
		return depth_first_search(p, p_proj, tree);
	}
	return std::make_tuple(std::numeric_limits<double>::quiet_NaN(), std::vector<size_t>(), std::vector<Point3>());
}

template<>
Point3 Octree<Segment>::project_closest(Point3& p, std::shared_ptr<TreeItem<Segment>>& tree)
{
	// project p on every face of the BB and choose the closest
	std::array<double, 6> pln_dist = {
		fabs(tree->bounds.lMin.x - p.x),
		fabs(tree->bounds.lMin.y - p.y),
		fabs(tree->bounds.lMin.z - p.z),
		fabs(tree->bounds.rMax.x - p.x),
		fabs(tree->bounds.rMax.y - p.y),
		fabs(tree->bounds.rMax.z - p.z) };
	auto it_min = std::min_element(pln_dist.begin(), pln_dist.end());
	double min_pln_dist = *it_min;
	auto min_pln_id = std::distance(pln_dist.begin(), it_min);

	Vec3 n{ { 0., 0., 0. } };
	n.v[min_pln_id % 3] = 1.;

	// а если они перпендикул€рны?
	double dist = n.dot(Vec3(p));
	Vec3 proj_vec = Vec3(p) - n * dist;
	Point3 p_proj = { proj_vec.v[0], proj_vec.v[1], proj_vec.v[2] };
	if (!root->bounds.is_inside(p_proj))
	{
		// distance to the nearest of 4 points of BB face
		double min_pt_dist = std::numeric_limits<double>::max();
		Point3 p_proj_new;

		auto face_points = tree->bounds.get_plane_points(min_pln_id);
		for_each(face_points.begin(), face_points.end(), [&](Point3& face_pt) {
			auto d = face_pt.euc_dist(p);
			if (min_pt_dist > d)
			{
				min_pt_dist = d;
				p_proj_new = face_pt;
			}
			});
		p_proj = p_proj_new;
	}
	return p_proj;
}

template<>
std::tuple<double, 
	std::vector<size_t>, 
	std::vector<Point3>>
	Octree<Segment>::depth_first_search(
		Point3& p, Point3& p_proj, std::shared_ptr<TreeItem<Segment>>& tree)
{
	std::vector<size_t> min_ids{};
	std::vector<Point3> min_proj{};

	double min_dist = std::numeric_limits<double>::max();

	size_t i_octant = 0;
	int point_octant = -1;

	for (auto& d_tree : tree->descendants)
	{
		if (d_tree->bounds.is_inside(p_proj))
		{
			const auto [d, ids, projs] = depth_first_search(p, p_proj, d_tree); 
			if (!std::isnan(d))
			{
				if (is_equal(d, min_dist))
				{
					min_ids.insert(min_ids.end(), ids.begin(), ids.end());
					min_proj.insert(min_proj.begin(), projs.begin(), projs.end());
				}
				if (d < min_dist)
				{
					min_dist = d;
					min_ids = ids;
					min_proj = projs;
				}
				
			}
			point_octant = i_octant;
		}
		++i_octant;
	}

	// check current tree data for closest segments
	for (auto& s : tree->data)
	{
		auto [d, p_proj]  = s->euc_dist(p);
		if (is_equal(d, min_dist))
		{
			min_ids.push_back(s->id);
			min_proj.push_back(p_proj);
		}
		if (d < min_dist)
		{
			min_dist = d;
			min_ids = std::vector<size_t>{ s->id };
			min_proj = std::vector<Point3>{ (p_proj) };
		}
	}

	// if the tree node has descendants, we may want to check 
	// those closest to the point p,
	// but only if the current min_dist > dist(F_i, p), where F_i -- is an internal plane between nodes
	if (tree->descendants.size())
	{
		size_t d_indx = 0;
		Point3 center = (tree->bounds.rMax - tree->bounds.lMin) * 0.5;
		double dist_xmid = fabs(center.x - p.x),
			dist_ymid = fabs(center.y - p.y),
			dist_zmid = fabs(center.z - p.z);


		auto search_neighbour = [&](size_t d_indx)
		{
			Point3 p_proj = project_closest(p, tree->descendants[d_indx]);
			// if the point p is closer to its projection p_proj onto BBox's plane
			// than to the closest segment, we may want to search the neighbour octant
			if (p_proj.euc_dist(p) < min_dist)
			{
				const auto [d, ids, projs] = depth_first_search(p, p_proj, tree->descendants[d_indx]);
				if (!std::isnan(d))
				{
					if (is_equal(d, min_dist))
					{
						min_ids.insert(min_ids.end(), ids.begin(), ids.end());
						min_proj.insert(min_proj.begin(), projs.begin(), projs.end());
					}
					if (d < min_dist)
					{
						min_dist = d;
						min_ids = ids;
						min_proj = projs;

					}
					
				}
			}
		};

		if (min_dist > std::min(std::min(dist_xmid, dist_ymid), dist_zmid))
		{
			if (min_dist > dist_xmid)
			{
				// 1->0; 3->2; 5->4; 7->6
				if (point_octant % 2)
				{
					d_indx = point_octant - 1;
				}
				// 0->1; 2->3; 4->5; 6->7;
				else
				{
					d_indx = point_octant + 1;
				}
				search_neighbour(d_indx);
			}
			if (min_dist > dist_ymid)
			{
				if (point_octant < 4)
				{
					d_indx = 3 - point_octant;
				}
				else
				{
					d_indx = 7 - point_octant;
				}
				search_neighbour(d_indx);
			}

			if (min_dist > dist_zmid)
			{
				if (point_octant < 4)
				{
					d_indx = point_octant + 4;
				}
				else
				{
					d_indx = point_octant - 4;
				}
				search_neighbour(d_indx);
			}	
		}
	}
	if (!min_ids.size())
		min_dist = std::numeric_limits<double>::quiet_NaN();
	return std::make_tuple(min_dist, min_ids, min_proj);

}

