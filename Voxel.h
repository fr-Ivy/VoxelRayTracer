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

	Voxel();
	~Voxel();
	void Set(const uint x, const uint y, const uint z, const uint v);
	void SetTransform(const mat4& t);
	void Intersect(Ray& ray);
	bool IsOccluded(Ray& ray);
	void BuildBrickGrid();
	void LoadFromFile(const char* file);
	unsigned int* grid; // voxel payload is 'unsigned int', interpretation of the bits is free!
	uint8_t* brickGrid; // uint8_t for faster reading

	float3 aabbMin, aabbMax;
	mat4 transform;
	mat4 invertedTransform;

private:
	bool Setup3DDDA(Ray& ray, DDAState& state) const;
	void UpdateBrick(uint bx, uint by, uint bz);

};

