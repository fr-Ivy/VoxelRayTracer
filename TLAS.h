#pragma once

struct TLASNode
{
	float3 aabbMin;
	uint leftRight;
	float3 aabbMax;
	uint BLAS;
	bool IsLeaf() const;
};

class TLAS
{
public:
	TLAS() = default;
	TLAS(Voxel* voxel, int N);
	void Build();
	void Intersect(Ray& ray) const;
	bool IsOccluded(Ray& ray) const;
	int FindBestMatch(const int* list, int N, int A) const;

private:
	TLASNode* tlasNode = 0;
	Voxel* blas;
	uint nodesUsed, blasCount;

};

