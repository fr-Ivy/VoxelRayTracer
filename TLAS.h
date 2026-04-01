#pragma once

struct TLASNode
{
	float3 aabbMin;
	uint leftRight;
	float3 aabbMax;
	uint BLAS;
	bool IsLeaf();
};

class TLAS
{
public:
	TLAS() = default;
	TLAS(Voxel* voxel, int N);
	void Build();
	void Intersect(Ray& ray);
	bool IsOccluded(Ray& ray);
	int FindBestMatch(int* list, int N, int A);

private:
	TLASNode* tlasNode = 0;
	Voxel* blas;
	uint nodesUsed, blasCount;

};

