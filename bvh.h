#pragma once

struct BVHNode
{
	float3 aabbMin, aabbMax;
	uint leftFirst;
	uint sphereCount;

	bool IsLeaf()
	{
		return sphereCount > 0;
	}
};

struct AABB
{
	float3 bmin = float3(1e30f);
	float3 bmax = float3(-1e30f);
	void Grow(float3 p);
	float Area();
};

class BVH
{
public:
	BVH() = default;
	void BuildBVH(Scene& scene);
	void UpdateNodeBounds(uint nodeIndex, const Scene& scene);
	void Subdivide(uint nodeIndex, const Scene& scene);
	void IntersectBVH(Ray& ray, const Scene& scene);
	float IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax);
	float EvaluateSAH(BVHNode& node, int axis, float pos, const Scene& scene);
	std::vector<BVHNode> bvhNodes;
	std::vector<uint> sphereIndex;
	int nodesUsed = 1;

private:

};

