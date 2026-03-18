#include "template.h"
#include "TLAS.h"

TLAS::TLAS(int N)
{
	blasCount = N;
	//call the grid
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
			//grid intersect
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
		//TLASNode* child1 = &TLASNode[node->leftBLAS];
		//TLASNode* child2 = &TLASNode[node->leftBLAS + 1];
	}
}
