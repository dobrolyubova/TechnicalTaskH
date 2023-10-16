#include "octree_item.h"
#include "polyline.h"

void TreeItem<Segment>::split()
{
	Point3 center = bounds.lMin + (bounds.rMax - bounds.lMin) * 0.5;
	
	descendants.resize(8);
	descendants[0] = std::make_shared <TreeItem>(AABBox{
		bounds.lMin, 
		center });
	descendants[1] = std::make_shared <TreeItem>(AABBox{
		Point3{center.x, bounds.lMin.y, bounds.lMin.z} ,
		Point3{bounds.rMax.x, center.y, center.z } });
	descendants[2] = std::make_shared <TreeItem>(AABBox{
		Point3{center.x, center.y, bounds.lMin.z} ,
		Point3{bounds.rMax.x, bounds.rMax.y, center.z } });
	descendants[3] = std::make_shared <TreeItem>(AABBox{
		Point3{bounds.lMin.x, center.y, bounds.lMin.z} ,
		Point3{center.x, bounds.rMax.y, center.z } });
	
	descendants[4] = std::make_shared <TreeItem>(AABBox{
		Point3{bounds.lMin.x, bounds.lMin.y, center.z} ,
		Point3{center.x, center.y, bounds.rMax.z } });
	descendants[5] = std::make_shared <TreeItem>(AABBox{
		Point3{center.x, bounds.lMin.y, center.z} ,
		Point3{bounds.rMax.x, center.y, bounds.rMax.z } });
	descendants[6] = std::make_shared <TreeItem>(AABBox{
		center,
		bounds.rMax });
	descendants[7] = std::make_shared <TreeItem>(AABBox{
		Point3{bounds.lMin.x, center.y, center.z} ,
		Point3{center.x, bounds.rMax.y, bounds.rMax.z } });
}

std::tuple<bool, size_t> TreeItem<Segment>::is_inside(Segment& obj)
{
	bool p1_inside = bounds.is_inside(*(obj.p1));
	bool p2_inside = bounds.is_inside(*(obj.p2));
	return std::make_tuple(p1_inside || p2_inside, p1_inside && p2_inside);
}


bool AABBox::is_inside(const Point3& p) const
{
	if (p.x < lMin.x || p.x > rMax.x) return false;
	if (p.y < lMin.y || p.y > rMax.y) return false;
	if (p.z < lMin.z || p.z > rMax.z) return false;
	return true;
}

//    4 | Z
//    --------- 7   
// 5/   |   6 /|
//  ---------  |
// |  0 ----|-|--3-- Y 
// |  /      |/
// 1 --------2
//  / X

std::array<Point3, 8> AABBox::get_all_points() const
{
	return std::array<Point3, 8>{
		lMin,
		Point3{ lMin.x, rMax.y, lMin.z },
		Point3{ rMax.x, rMax.y, lMin.z },
		Point3{ rMax.x, lMin.y, lMin.z },
			
		Point3{ lMin.x, rMax.y, rMax.z },
		Point3{ rMax.x, rMax.y, rMax.z },
		rMax,
		Point3{ rMax.x, lMin.y, rMax.z }
	};
}

//    .5| Z
//    ---------    
// 1/   |   0 /|
//  ---------  |
// | .3/ ----|-|---- Y 
// |  / 2    | /4
// -----------
//X/

std::array<Point3, 4> AABBox::get_plane_points(size_t id) const
{

	switch (id)
	{
	default: return std::array<Point3, 4>{};
	case 0: {
		return std::array<Point3, 4>{
			lMin,
				Point3{ lMin.x, rMax.y, lMin.z },
				Point3{ lMin.x, rMax.y, rMax.z },
				Point3{ lMin.x, lMin.y, rMax.z }
		};
	}
	case 1: {
		return std::array<Point3, 4>{
			lMin,
				Point3{ rMax.x, lMin.y, lMin.z },
				Point3{ lMin.x, lMin.y, rMax.z },
				Point3{ rMax.x, lMin.y, rMax.z }
		};
	}
	case 2: {
		return std::array<Point3, 4>{
			lMin,
				Point3{ rMax.x, lMin.y, lMin.z },
				Point3{ rMax.x, rMax.y, lMin.z },
				Point3{ lMin.x, rMax.y, lMin.z }
		};
	}
	case 3: {
		return std::array<Point3, 4>{
			    Point3{ rMax.x, lMin.y, lMin.z },
				Point3{ rMax.x, rMax.y, lMin.z },
				rMax,
				Point3{ rMax.x, lMin.y, rMax.z }
		};
	}
	case 4: {
		return std::array<Point3, 4>{
				Point3{ lMin.x, rMax.y, lMin.z },
				Point3{ rMax.x, rMax.y, lMin.z },
				rMax,
				Point3{ lMin.x, rMax.y, rMax.z }
		};
	}
	case 5: {
		return std::array<Point3, 4>{
				Point3{ lMin.x, lMin.y, rMax.z }, 
				Point3{ rMax.x, lMin.y, rMax.z },
				rMax,
				Point3{ lMin.x, rMax.y, rMax.z }
		};
	}
	}
}

template<>
bool AABBox::is_inside(const Segment& s) const
{
	return is_inside(*s.p1) && is_inside(*s.p2);
}
