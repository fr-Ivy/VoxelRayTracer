#include "template.h"
#include "TLAS.h"

#include "Voxel.h"

TLAS::TLAS(Voxel* voxel, int N)
{
	blas = voxel;
	blasCount = N;
	tlasNode = (TLASNode*)_aligned_malloc(sizeof(TLASNode) * 2 * N, 64);
	nodesUsed = 2;
}

void TLAS::Build()
{
	tlasNode[2].leftBLAS = 0;
	tlasNode[2].aabbMin = float3(-100);
	tlasNode[2].aabbMax = float3(100);
	tlasNode[2].isLeaf = true;
	tlasNode[3].leftBLAS = 1;
	tlasNode[3].aabbMin = float3(-100);
	tlasNode[3].aabbMax = float3(100);
	tlasNode[3].isLeaf = true;

	tlasNode[0].leftBLAS = 2;
	tlasNode[0].aabbMin = float3(-100);
	tlasNode[0].aabbMax = float3(100);
	tlasNode[0].isLeaf = false;
}

void TLAS::Intersect(Ray& ray)
{
	TLASNode* node = &tlasNode[0];
	TLASNode* stack[64];
	uint stackPtr = 0;
	while (true)
	{
		if (node->isLeaf)
		{
			blas->Intersect(ray);
			if (stackPtr == 0)
			{
				break;
			}
			else
			{
				node = stack[--stackPtr];
			}
			continue;
		}
		TLASNode* child1 = &tlasNode[node->leftBLAS];
		TLASNode* child2 = &tlasNode[node->leftBLAS + 1];

		float distance1, distance2;
		IntersectAABB(ray, child1->aabbMin, child1->aabbMax, child2->aabbMin, child2->aabbMax, distance1, distance2);

		if (distance1 > distance2)
		{
			swap(distance1, distance2);
			swap(child1, child2);
		}

		if (distance1 == 1e30f)
		{
			if (stackPtr == 0)
			{
				break;
			}
			else
			{
				node = stack[stackPtr--];
			}
		}
		else
		{
			node = child1;
			if (distance2 != 1e30f)
			{
				stack[stackPtr++] = child2;
			}
		}
	}
}
