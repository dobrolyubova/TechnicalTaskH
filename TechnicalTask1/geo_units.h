#pragma once
#include <array>

static bool is_equal(const double x, const double y)
{
	return fabs(x - y) <= std::numeric_limits<double>::epsilon();
}

// 3D point and 3D vector with basic operators overload
namespace geo_units
{

	struct Point3
	{
		double x, y, z;

		bool operator == (const Point3& p) const
		{
			return is_equal(x, p.x) && is_equal(y, p.y) && is_equal(z, p.z);
		}
		Point3 operator + (const Point3& p) const
		{
			return Point3{ x + p.x, y + p.y, z + p.z };
		}
		Point3 operator - (const Point3& p) const
		{
			return Point3{ x - p.x, y - p.y, z - p.z };
		}
		Point3 operator * (const double& a) const
		{
			return Point3{ a * x, a * y, a * z };
		}

		double euc_dist(const Point3& p) const
		{
			Point3 diff{x - p.x, y-p.y, z-p.z};
			return sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
		}
	};

	struct Vec3
	{
		std::array<double, 3> v;

		Vec3(const Point3& p)
		{
			v[0] = p.x; v[1] = p.y; v[2] = p.z;
		}

		Vec3 operator + (const Vec3& p) const
		{
			return Vec3{ { v[0] + p.v[0], v[1] + p.v[1], v[2] + p.v[2]} };
		}
		Vec3 operator * (const double& a) const
		{
			return Vec3{ { a * v[0], a * v[1], a * v[2] } };
		}
		Vec3 operator - (const Vec3& p) const
		{
			return *this + p * (-1);
		}
		double dot(const Vec3& p) const
		{
			return v[0] * p.v[0] + v[1] * p.v[1] + v[2] * p.v[2];
		}

		Vec3 cross(const Vec3& p) const
		{
			return Vec3{
				{
					v[1] * p.v[2] - v[2] * p.v[1],
					v[2] * p.v[0] - v[0] * p.v[2],
					v[0] * p.v[1] - v[1] * p.v[0]
				}
			};
		}

		double norm() const
		{
			return sqrt((this->dot(*this)));
		}
	};
}