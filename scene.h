#pragma once

// high level settings
//128 recommended
#define WORLDSIZE 512 // power of 2. Warning: max 512 for a 512x512x512x4 bytes = 512MB world!

// low-level / derived
#define WORLDSIZE2	(WORLDSIZE*WORLDSIZE)
#define WORLDSIZE3	(WORLDSIZE*WORLDSIZE*WORLDSIZE)

//MLG
#define BRICKSIZE 16 // power of 2. voxels per brick side
#define BRICKGRID (WORLDSIZE / BRICKSIZE)
#define BRICKGRID2 (BRICKGRID * BRICKGRID)
#define BRICKGRID3 (BRICKGRID * BRICKGRID * BRICKGRID)

// epsilon
#define EPSILON		0.00001f

#include <immintrin.h>


class BVH;
class Voxel;
class TLAS;

struct Sphere
{
	float3 center;
	float radius;
	uint material;
	float3 velocity;
	float3 previousCenter;
};

namespace Tmpl8 {

	class Scene
	{
	public:
		std::vector<Sphere> spheres;

		Scene();
		~Scene();
		void FindNearest(Ray& ray, bool skipBVH = false) const;
		bool IsOccluded(Ray& ray) const;

		void SetSphere(float3 center, float radius, uint material);

		bool shadows = true;
		BVH* bvh;
		Voxel* voxelWorld;
		TLAS* tlas;
	private:

	};


	inline float intersect_cube(Ray& ray)
	{
		// branchless slab method by Tavian
		const float tx1 = -ray.O.x * ray.rD.x, tx2 = (1 - ray.O.x) * ray.rD.x;
		float ty, tz, tmin = min(tx1, tx2), tmax = max(tx1, tx2);
		const float ty1 = -ray.O.y * ray.rD.y, ty2 = (1 - ray.O.y) * ray.rD.y;
		ty = min(ty1, ty2), tmin = max(tmin, ty), tmax = min(tmax, max(ty1, ty2));
		const float tz1 = -ray.O.z * ray.rD.z, tz2 = (1 - ray.O.z) * ray.rD.z;
		tz = min(tz1, tz2), tmin = max(tmin, tz), tmax = min(tmax, max(tz1, tz2));
		if (tmin == tz) ray.axis = 2; else if (tmin == ty) ray.axis = 1;
		return tmax >= tmin && tmin > 0 ? tmin : 1e34f;
	}

	inline bool point_in_cube(const float3& pos)
	{
		// test if pos is inside the cube
		return pos.x >= 0 && pos.y >= 0 && pos.z >= 0 &&
			pos.x <= 1 && pos.y <= 1 && pos.z <= 1;
	}

	inline float IntersectAABB(const Ray& ray, const float3& bmin, float3& bmax)
	{
		float tx1 = (bmin.x - ray.O.x) / ray.D.x, tx2 = (bmax.x - ray.O.x) / ray.D.x;
		float tmin = min(tx1, tx2), tmax = max(tx1, tx2);
		float ty1 = (bmin.y - ray.O.y) / ray.D.y, ty2 = (bmax.y - ray.O.y) / ray.D.y;
		tmin = max(tmin, min(ty1, ty2)), tmax = min(tmax, max(ty1, ty2));
		float tz1 = (bmin.z - ray.O.z) / ray.D.z, tz2 = (bmax.z - ray.O.z) / ray.D.z;
		tmin = max(tmin, min(tz1, tz2)), tmax = min(tmax, max(tz1, tz2));
		if (tmax >= tmin && tmin < ray.t && tmax > 0) return tmin; else return 1e30f;
	}


	_forceinline float IntersectSphere(Ray& ray, const Sphere& sphere)
	{
		float3 c = sphere.center - ray.O;
		float t = dot(c, ray.D);
		float3 q = c - t * ray.D;
		float p2 = dot(q, q);
		float radius2 = pow2f(sphere.radius);
		if (p2 > radius2)
		{
			return 1e34f;
		}

		t -= sqrt(radius2 - p2);
		if ((t > 0))
		{
			return t;
		}
		return 1e34f;
	}

	inline void IntersectAABB(const Ray& ray, const float3& bmin1, const float3& bmax1, const float3& bmin2, const float3& bmax2, float& d1, float& d2)
	{
		// branchless slab method by Tavian
		__m128 tx = _mm_mul_ps(_mm_sub_ps(_mm_set_ps(bmax2.x, bmin2.x, bmax1.x, bmin1.x), _mm_set1_ps(ray.O.x)), _mm_set1_ps(ray.rD.x));
		__m128 ty = _mm_mul_ps(_mm_sub_ps(_mm_set_ps(bmax2.y, bmin2.y, bmax1.y, bmin1.y), _mm_set1_ps(ray.O.y)), _mm_set1_ps(ray.rD.y));
		__m128 tz = _mm_mul_ps(_mm_sub_ps(_mm_set_ps(bmax2.z, bmin2.z, bmax1.z, bmin1.z), _mm_set1_ps(ray.O.z)), _mm_set1_ps(ray.rD.z));

		const int swap = _MM_SHUFFLE(2, 3, 0, 1);
		__m128 tmin = _mm_max_ps(_mm_max_ps(_mm_min_ps(tx, _mm_shuffle_ps(tx, tx, swap)),
			_mm_min_ps(ty, _mm_shuffle_ps(ty, ty, swap))),
			_mm_min_ps(tz, _mm_shuffle_ps(tz, tz, swap)));
		__m128 tmax = _mm_min_ps(_mm_min_ps(_mm_max_ps(tx, _mm_shuffle_ps(tx, tx, swap)),
			_mm_max_ps(ty, _mm_shuffle_ps(ty, ty, swap))),
			_mm_max_ps(tz, _mm_shuffle_ps(tz, tz, swap)));

		float tmin1 = _mm_cvtss_f32(tmin);
		float tmax1 = _mm_cvtss_f32(tmax);
		float tmin2 = _mm_cvtss_f32(_mm_shuffle_ps(tmin, tmin, _MM_SHUFFLE(0, 0, 0, 2)));
		float tmax2 = _mm_cvtss_f32(_mm_shuffle_ps(tmax, tmax, _MM_SHUFFLE(0, 0, 0, 2)));

		d1 = (tmax1 >= tmin1 && tmin1 < ray.t && tmax1 > 0) ? tmin1 : 1e34f;
		d2 = (tmax2 >= tmin2 && tmin2 < ray.t && tmax2 > 0) ? tmin2 : 1e34f;
	}

}