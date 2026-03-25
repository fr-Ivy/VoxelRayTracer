#pragma once
class Voxel
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

	struct SEGMENT
	{
		float3 a;
		float3 b;
		float3 c;
		float3 d;
		float duration;
	};

	std::vector<SEGMENT> splineSegments;
	float splineTime = 0.0f;

	uint worldSize = WORLDSIZE;
	uint worldSize2 = WORLDSIZE2;
	uint worldSize3 = WORLDSIZE3;
	uint brickGridSize = BRICKGRID;
	uint brickGridSize2 = BRICKGRID2;
	uint brickGridSize3 = BRICKGRID3;

	Voxel();
	~Voxel();
	void Resize(uint size);
	void Set(const uint x, const uint y, const uint z, const uint v);
	void SetTransform(const mat4& t);
	void Intersect(Ray& ray);
	bool IsOccluded(Ray& ray);
	void BuildBrickGrid();
	void LoadFromFile(const char* file);
	void AddSplineSegment(float3 p0, float3 p1, float3 p2, float3 p3, float duration);
	float3 EvaluateSpline(float t);
	void UpdateSpline(float deltaTime);
	

	unsigned int* grid; // voxel payload is 'unsigned int', interpretation of the bits is free!
	uint8_t* brickGrid; // uint8_t for faster reading

	float3 aabbMin, aabbMax;
	mat4 transform;
	mat4 invertedTransform;
	float gridScale = 1.0f;

private:
	bool Setup3DDDA(Ray& ray, DDAState& state) const;
	void UpdateBrick(uint bx, uint by, uint bz);

};

