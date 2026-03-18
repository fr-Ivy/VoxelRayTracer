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
	TLAS(/*call the grid*/ int N);
	void Build();
	void Intersect(Ray& ray);

private:
	TLASNode* tlasNode = 0;
	//call the grid
	uint nodesUsed, blasCount;

};

