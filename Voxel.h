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
	void Intersect(Ray& ray);
	bool IsOccluded(Ray& ray);
	void BuildBrickGrid();
	unsigned int* grid; // voxel payload is 'unsigned int', interpretation of the bits is free!
	uint8_t* brickGrid; // uint8_t for faster reading

private:
	bool Setup3DDDA(Ray& ray, DDAState& state) const;

	void UpdateBrick(uint bx, uint by, uint bz);

};

