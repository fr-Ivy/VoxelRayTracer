#include "template.h"
#include "TLAS.h"

#include "Voxel.h"

bool TLASNode::IsLeaf() const
{
	return leftRight == 0;
}

TLAS::TLAS(Voxel* voxel, int N)
{
	blas = voxel;
	blasCount = N;
	tlasNode = (TLASNode*)_aligned_malloc(sizeof(TLASNode) * 2 * N, 64);
	nodesUsed = 2;
}

void TLAS::Build()
{
	// assign a TLASleaf node each BLAS
	int nodeIndex[256];
	int nodeIndices = blasCount;
	nodesUsed = 1;

	for (uint i = 0; i < blasCount; i++)
	{
		nodeIndex[i] = nodesUsed;
		tlasNode[nodesUsed].aabbMin = blas[i].aabbMin;
		tlasNode[nodesUsed].aabbMax = blas[i].aabbMax;
		tlasNode[nodesUsed].BLAS = i;
		tlasNode[nodesUsed++].leftRight = 0; // makes it a leaf
	}

	// use agglomerative clustering to build the TLAS
	// repeatedly merge the two closest nodes until only one node is left.
	int A = 0;
	int B = FindBestMatch(nodeIndex, nodeIndices, A);
	while (nodeIndices > 1)
	{
		int C = FindBestMatch(nodeIndex, nodeIndices, B);
		if (A == C)
		{
			int const nodeIndexA = nodeIndex[A];
			int const nodeIndexB = nodeIndex[B];
			TLASNode& nodeA = tlasNode[nodeIndexA];
			TLASNode& nodeB = tlasNode[nodeIndexB];
			TLASNode& newNode = tlasNode[nodesUsed];
			newNode.leftRight = nodeIndexA + (nodeIndexB << 16);
			newNode.aabbMin = fminf(nodeA.aabbMin, nodeB.aabbMin);
			newNode.aabbMax = fmaxf(nodeA.aabbMax, nodeB.aabbMax);
			nodeIndex[A] = nodesUsed++;
			nodeIndex[B] = nodeIndex[nodeIndices - 1];
			B = FindBestMatch(nodeIndex, --nodeIndices, A);
		}
		else
		{
			A = B;
			B = C;
		}
	}
	tlasNode[0] = tlasNode[nodeIndex[A]];
}

void TLAS::Intersect(Ray& ray) const
{
	// traverse the TLAS
	const TLASNode* node = &tlasNode[0];
	const TLASNode* stack[64];
	uint stackPtr = 0;
	while (true)
	{
		// if leaf node, intersect the ray with the BLAS.
		if (node->IsLeaf())
		{
			blas[node->BLAS].Intersect(ray);
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
		TLASNode* child1 = &tlasNode[node->leftRight &0xffff];
		TLASNode* child2 = &tlasNode[node->leftRight >> 16];

		float distance1, distance2;
		IntersectAABB(ray, child1->aabbMin, child1->aabbMax, child2->aabbMin, child2->aabbMax, distance1, distance2);

		if (distance1 > distance2)
		{
			swap(distance1, distance2);
			swap(child1, child2);
		}

		// if the ray misses both children, then break.
		if (distance1 == 1e30f)
		{
			if (stackPtr == 0)
			{
				break;
			}
			else
			{
				node = stack[--stackPtr];
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

bool TLAS::IsOccluded(Ray& ray) const
{
	// traverse the TLAS
	const TLASNode* node = &tlasNode[0];
	const TLASNode* stack[64];
	uint stackPtr = 0;
	while (true)
	{
		// if leaf node, intersect the ray with the BLAS.
		if (node->IsLeaf())
		{
			if (blas[node->BLAS].IsOccluded(ray))
			{
				return true;
			}
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
		TLASNode* child1 = &tlasNode[node->leftRight & 0xffff];
		TLASNode* child2 = &tlasNode[node->leftRight >> 16];

		float distance1, distance2;
		IntersectAABB(ray, child1->aabbMin, child1->aabbMax, child2->aabbMin, child2->aabbMax, distance1, distance2);

		if (distance1 > distance2)
		{
			swap(distance1, distance2);
			swap(child1, child2);
		}

		// if the ray misses both children, then break.
		if (distance1 == 1e30f)
		{
			if (stackPtr == 0)
			{
				break;
			}
			else
			{
				node = stack[--stackPtr];
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
	return false;
}

int TLAS::FindBestMatch(const int* list, int const N, int const A) const
{
	// find the BLAS that has the smallest surface area when merged with A.
	float smallest = 1e30f;
	int bestB = -1;
	for (int B = 0; B < N; B++)
	{
		if (B != A)
		{
			float3 bmax = fmaxf(tlasNode[list[A]].aabbMax, tlasNode[list[B]].aabbMax);
			float3 bmin = fminf(tlasNode[list[A]].aabbMin, tlasNode[list[B]].aabbMin);
			float3 const e = bmax - bmin;
			float surfaceArea = e.x * e.y + e.y * e.z + e.z * e.x;
			if (surfaceArea < smallest)
			{
				smallest = surfaceArea;
				bestB = B;
			}
		}
	}
	return bestB;
}
