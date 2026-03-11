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

class BVH;

struct Sphere
{
	float3 center;
	float radius;
	uint material;
	float3 velocity;
};

namespace Tmpl8 {

class Scene
{
public:
	struct DDAState
	{
		int3 step;
		uint X, Y, Z;
		float t;
		float3 tdelta;
		float3 tmax;
	};

	std::vector<Sphere> spheres;

	Scene();
	~Scene();
	void FindNearest( Ray& ray ) const;
	bool IsOccluded( Ray& ray ) const;
	void Set( const uint x, const uint y, const uint z, const uint v );
	void SetSphere(float3 center, float radius, uint material);
	unsigned int* grid; // voxel payload is 'unsigned int', interpretation of the bits is free!
	uint8_t* brickGrid; // uint8_t for faster reading
	//float3 sphereCenter;
	//float sphereRadius;
	//uint sphereMaterial;
	BVH* bvh;
private:
	bool Setup3DDDA( Ray& ray, DDAState& state ) const;
	void BuildBrickGrid();
	void UpdateBrick(uint bx, uint by, uint bz);
};

inline float IntersectSphere(Ray& ray, const Sphere& sphere)
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

}