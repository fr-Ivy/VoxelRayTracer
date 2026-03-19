#pragma once

struct TLASNode
{
	float3 aabbMin;
	uint leftBLAS;
	float3 aabbMax;
	uint isLeaf;
};

class TLAS
{
public:
	TLAS() = default;
	TLAS(Voxel* voxel, int N);
	void Build();
	void Intersect(Ray& ray);

private:
	TLASNode* tlasNode = 0;
	Voxel* blas;
	uint nodesUsed, blasCount;

};

