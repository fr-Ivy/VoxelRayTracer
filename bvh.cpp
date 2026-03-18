#include "template.h"
#include "bvh.h"
#include "scene.h"

void AABB::Grow(float3 p)
{
	// clamp p to avoid issues
	bmin = fminf(bmin, p);
	bmax = fmaxf(bmax, p);
}

float AABB::Area()
{
	// Surface area of the AABB, later used for SAH calculations
	float3 e = bmax - bmin;
	return e.x * e.y + e.y * e.z + e.z * e.x;
}

void BVH::BuildBVH(Scene& scene)
{
	// build the BVH 
	nodesUsed = 1;
	int sphereCount = static_cast<int>(scene.spheres.size());
	if (sphereCount == 0)
	{
		return;
	}

	// resize so that it can take spheres and nodes
	bvhNodes.clear();
	bvhNodes.resize(sphereCount * 2 - 1);
	sphereIndex.clear();
	sphereIndex.resize(sphereCount);
	for (int i = 0; i < sphereCount; i++)
	{
		sphereIndex[i] = i;
	}

	// initialize root node
	bvhNodes[0].leftFirst = 0;
	bvhNodes[0].sphereCount = sphereCount;
	UpdateNodeBounds(0, scene);
	Subdivide(0, scene);
}

void BVH::UpdateNodeBounds(uint nodeIndex, const Scene& scene)
{
	// calculate the AABB of the nodes.
	BVHNode& node = bvhNodes[nodeIndex];
	node.aabbMin = float3(1e30f);
	node.aabbMax = float3(-1e30f);

	// loop over the spheres in the node and grow the AABB to fit them
	for (uint i = 0; i < node.sphereCount; i++)
	{
		const Sphere& sphere = scene.spheres[sphereIndex[node.leftFirst + i]];

		//AABB
		node.aabbMin = fminf(node.aabbMin, sphere.center - sphere.radius);
		node.aabbMax = fmaxf(node.aabbMax, sphere.center + sphere.radius);
	}
}

void BVH::Subdivide(uint nodeIndex, const Scene& scene)
{
	// Subdivide the node using a binned SAH approach.
	BVHNode& node = bvhNodes[nodeIndex];

	const int BINS = 8;

	int bestAxis = -1;
	float splitPos = 0;
	float bestCost = 1e30f;

	for (int axis = 0; axis < 3; axis++)
	{
		float boundsMin = 1e30f;
		float boundsMax = -1e30f;

		// find the bounds of the spheres in this node along the right axis.
		for (uint i = 0; i < node.sphereCount; i++)
		{
			const Sphere& sphere = scene.spheres[sphereIndex[node.leftFirst + i]];
			boundsMin = min(boundsMin, sphere.center[axis]);
			boundsMax = max(boundsMax, sphere.center[axis]);
		}

		// if the bounds are equal, skip this axis, because it can't be split along this axis.
		if (boundsMin == boundsMax)
		{
			continue;
		}

		// initialize bins
		struct Bin
		{
			AABB bounds;
			int count = 0;
		};
		Bin bins[BINS];
		float scale = BINS / (boundsMax - boundsMin);

		// loop over the spheres in the node and put them in the bins
		for (uint i = 0; i < node.sphereCount; i++)
		{
			const Sphere& sphere = scene.spheres[sphereIndex[node.leftFirst + i]];
			int binIndex = min(BINS - 1, max(0, static_cast<int>((sphere.center[axis] - boundsMin) * scale)));
			bins[binIndex].count++;
			bins[binIndex].bounds.Grow(sphere.center - sphere.radius);
			bins[binIndex].bounds.Grow(sphere.center + sphere.radius);
		}

		float leftArea[BINS - 1];
		float rightArea[BINS - 1];
		int leftCount[BINS - 1];
		int rightCount[BINS - 1];

		AABB leftBox, rightBox;
		int leftSum = 0;
		int rightSum = 0;

		// calculate the sums and areas for the left and right sides of the split.
		for (int i = 0; i < BINS - 1; i++)
		{
			leftSum += bins[i].count;
			leftCount[i] = leftSum;
			if (bins[i].count > 0)
			{
				leftBox.Grow(bins[i].bounds.bmin);
				leftBox.Grow(bins[i].bounds.bmax);
			}
			leftArea[i] = leftBox.Area();

			rightSum += bins[BINS - 1 - i].count;
			rightCount[BINS - 2 - i] = rightSum;
			if (bins[BINS - 1 - i].count > 0)
			{
				rightBox.Grow(bins[BINS - 1 - i].bounds.bmin);
				rightBox.Grow(bins[BINS - 1 - i].bounds.bmax);
			}
			rightArea[BINS - 2 - i] = rightBox.Area();
		}

		// calculate the cost of splitting after each bin and find the best split.
		scale = (boundsMax - boundsMin) / BINS;
		for (uint i = 0; i < BINS - 1; i++)
		{
			float cost = leftCount[i] * leftArea[i] + rightCount[i] * rightArea[i];
			if (cost < bestCost)
			{
				bestCost = cost;
				bestAxis = axis;
				splitPos = boundsMin + scale * (i + 1);
			}
		}
	}

	// if no split was found, return.
	float3 extent = node.aabbMax - node.aabbMin;
	float parentArea = extent.x * extent.y + extent.y * extent.z + extent.z * extent.x;
	float parentCost = node.sphereCount * parentArea;
	if (bestCost >= parentCost)
	{
		return;
	}

	// seperate the spheres in the node into two groups based on their best split and create child nodes for them.
	int i = static_cast<int>(node.leftFirst);
	int j = i + static_cast<int>(node.sphereCount - 1);

	while (i <= j)
	{
		if (scene.spheres[sphereIndex[i]].center[bestAxis] < splitPos)
		{
			i++;
		}
		else
		{
			swap(sphereIndex[i], sphereIndex[j--]);
		}
	}

	int leftCount = i - node.leftFirst;
	if (leftCount == 0 || leftCount == node.sphereCount)
	{
		return;
	}

	int leftChildIndex = nodesUsed++;
	int rightChildIndex = nodesUsed++;

	bvhNodes[leftChildIndex].leftFirst = node.leftFirst;
	bvhNodes[leftChildIndex].sphereCount = leftCount;
	bvhNodes[rightChildIndex].leftFirst = i;
	bvhNodes[rightChildIndex].sphereCount = node.sphereCount - leftCount;

	node.leftFirst = leftChildIndex;
	node.sphereCount = 0;

	// update the bounds of the child nodes and subdivide them.
	UpdateNodeBounds(leftChildIndex, scene);
	UpdateNodeBounds(rightChildIndex, scene);
	Subdivide(leftChildIndex, scene);
	Subdivide(rightChildIndex, scene);
}

void BVH::IntersectBVH(Ray& ray, const Scene& scene)
{
	// if the BVH is empty, return.
	if (bvhNodes.empty())
	{
		return;
	}

	BVHNode* node = &bvhNodes[0];
	BVHNode* stack[64];
	uint stackPtr = 0;

	// traverse the BVH and check for sphere intersection in the leaf nodes. If an intersection is found, update the ray with the intersection information.
	while (true)
	{
		if (node->IsLeaf())
		{
			for (uint i = 0; i < node->sphereCount; i++)
			{
				const Sphere& sphere = scene.spheres[sphereIndex[node->leftFirst + i]];
				float distance = IntersectSphere(ray, sphere);
				if (distance < ray.t)
				{
					ray.t = distance;
					ray.voxel = sphere.material;
					ray.hitSphere = true;
					ray.N = normalize((ray.O + distance * ray.D) - sphere.center);
				}
			}
			
			if (stackPtr == 0)
			{
				break;
			}
			node = stack[--stackPtr];
		}
		else
		{
			BVHNode* child1 = &bvhNodes[node->leftFirst];
			BVHNode* child2 = &bvhNodes[node->leftFirst + 1];

			float distance1 = IntersectAABB(ray, child1->aabbMin, child1->aabbMax);
			float distance2 = IntersectAABB(ray, child2->aabbMin, child2->aabbMax);

			if (distance1 > distance2)
			{
				swap(distance1, distance2);
				swap(child1, child2);
			}
			if (distance1 == 1e34f)
			{
				if (stackPtr == 0)
				{
					break;
				}
				node = stack[--stackPtr];
			}
			else
			{
				node = child1;
				if (distance2 != 1e34f)
				{
					stack[stackPtr++] = child2;
				}
			}
		}
	}
}

float BVH::IntersectAABB(const Ray& ray, const float3 bmin, const float3 bmax)
{
	// branchless slab method by Tavian
	float tx1 = (bmin.x - ray.O.x) * ray.rD.x;
	float tx2 = (bmax.x - ray.O.x) * ray.rD.x;
	float tmin = min(tx1, tx2);
	float tmax = max(tx1, tx2);

	float ty1 = (bmin.y - ray.O.y) * ray.rD.y;
	float ty2 = (bmax.y - ray.O.y) * ray.rD.y;
	tmin = max(tmin, min(ty1, ty2));
	tmax = min(tmax, max(ty1, ty2));

	float tz1 = (bmin.z - ray.O.z) * ray.rD.z;
	float tz2 = (bmax.z - ray.O.z) * ray.rD.z;
	tmin = max(tmin, min(tz1, tz2));
	tmax = min(tmax, max(tz1, tz2));

	if (tmax >= tmin && tmin < ray.t && tmax > 0)
	{
		return tmin;
	}
	return 1e34f;
}

float BVH::EvaluateSAH(BVHNode& node, int axis, float pos, const Scene& scene)
{
	// evaluate the SAH cost of splitting the node along the current axis at the given position.
	AABB leftBox, rightBox;
	int leftCount = 0;
	int rightCount = 0;

	for (uint i = 0; i < node.sphereCount; i++)
	{
		const Sphere& sphere = scene.spheres[sphereIndex[node.leftFirst + i]];

		if (sphere.center[axis] < pos)
		{
			leftCount++;
			leftBox.Grow(sphere.center - sphere.radius);
			leftBox.Grow(sphere.center + sphere.radius);
		}
		else
		{
			rightCount++;
			rightBox.Grow(sphere.center - sphere.radius);
			rightBox.Grow(sphere.center + sphere.radius);
		}
	}
	float cost = static_cast<float>(leftCount) * leftBox.Area() + static_cast<float>(rightCount) * rightBox.Area();
	return cost > 0 ? cost : 1e34f;
}
