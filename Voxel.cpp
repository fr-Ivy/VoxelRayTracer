#include "template.h"
#include "Voxel.h"

Voxel::Voxel()
{
	grid = nullptr;
	brickGrid = nullptr;
}

Voxel::~Voxel()
{
	if (ownsGrid)
	{
		FREE64(grid);
		FREE64(brickGrid);
	}
}

void Voxel::Resize(uint size)
{
	FREE64(grid);
	FREE64(brickGrid);

	// allocate room for the world
	worldSize = size;
	worldSize2 = size * size;
	worldSize3 = size * size * size;
	brickGridSize = size / BRICKSIZE;
	brickGridSize2 = brickGridSize * brickGridSize;
	brickGridSize3 = brickGridSize * brickGridSize * brickGridSize;

	gridScale = static_cast<float>(size) / 512.0f;

	grid = (uint*)MALLOC64(worldSize3 * sizeof(uint));
	memset(grid, 0, worldSize3 * sizeof(uint));
	brickGrid = (uint8_t*)MALLOC64(brickGridSize3 * sizeof(uint8_t));
	SetTransform(mat4::Scale(float3(gridScale)));
}

void Voxel::Set(const uint x, const uint y, const uint z, const uint v)
{
	const uint old = grid[x + y * worldSize + z * worldSize2];
	grid[x + y * worldSize + z * worldSize2] = v;

	if (!brickGrid || old == v)
	{
		return;
	}
	const uint brickX = x / BRICKSIZE;
	const uint brickY = y / BRICKSIZE;
	const uint brickZ = z / BRICKSIZE;

	if (v != 0)
	{
		brickGrid[brickX + brickY * brickGridSize + brickZ * brickGridSize2] = 1;
	}
	else
	{
		UpdateBrick(brickX, brickY, brickZ);
	}
}

void Voxel::SetTransform(const mat4& t)
{
	transform = t;
	invertedTransform = t.Inverted();

	aabbMin = float3(1e34f); // LARGE_FLOAT
	aabbMax = float3(-1e34f);

	for (int i = 0; i < 8; i++)
	{
		float3 corner = float3(i & 1 ? 1.0f : 0.0f, i & 2 ? 1.0f : 0.0f, i & 4 ? 1.0f : 0.0f);
		float3 world = TransformPosition(corner, transform);
		aabbMin = fminf(aabbMin, world);
		aabbMax = fmaxf(aabbMax, world);
	}
}

bool Voxel::Setup3DDDA(Ray& ray, DDAState& state) const
{
	// if ray is not inside the world: advance until it is
	state.t = 0;
	bool startedInGrid = point_in_cube(ray.O);
	if (!startedInGrid)
	{
		state.t = intersect_cube(ray);
		if (state.t > 1e33f) return false; // ray misses voxel data entirely
	}
	// setup amanatides & woo - assume world is 1x1x1, from (0,0,0) to (1,1,1)
	const float cellSize = 1.0f / worldSize;
	state.step = make_int3(1.0f - ray.Dsign * 2.0f);
	const float3 posInGrid = (float)worldSize * (ray.O + (state.t + 0.00005f) * ray.D);
	const float3 gridPlanes = (ceilf(posInGrid) - ray.Dsign) * cellSize;
	const int3 P = clamp(make_int3(posInGrid), 0, worldSize - 1);
	state.X = P.x, state.Y = P.y, state.Z = P.z;
	state.tdelta = cellSize * float3(state.step) * ray.rD;
	state.tmax = (gridPlanes - ray.O) * ray.rD;
	// detect rays that start inside a voxel
	uint cell = grid[P.x + P.y * worldSize + P.z * worldSize2];
	ray.inside = cell != 0 && startedInGrid;
	// proceed with traversal
	return true;
}

void Voxel::BuildBrickGrid()
{
	memset(brickGrid, 0, brickGridSize3 * sizeof(uint8_t)); // makes sure that the whole grid starts empty
	for (uint bz = 0; bz < brickGridSize; bz++)
	{
		for (uint by = 0; by < brickGridSize; by++)
		{
			for (uint bx = 0; bx < brickGridSize; bx++)
			{
				uint occupied = 0;

				const uint vx = bx * BRICKSIZE;
				const uint vy = by * BRICKSIZE;
				const uint vz = bz * BRICKSIZE;

				//walks over voxels inside of the brick
				for (uint lz = 0; lz < BRICKSIZE; lz++)
				{
					for (uint ly = 0; ly < BRICKSIZE; ly++)
					{
						for (uint lx = 0; lx < BRICKSIZE; lx++)
						{
							occupied |= grid[(vx + lx) + (vy + ly) * worldSize + (vz + lz) * worldSize2];
						}
					}
				}
				// if occupied is 1, at least one voxel was solid inside the brick
				brickGrid[bx + by * brickGridSize + bz * brickGridSize2] = occupied ? 1 : 0;
			}
		}
	}
}

void Voxel::LoadFromFile(const char* file)
{
	FILE* f = fopen(file, "rb");

	int sizeX, sizeY, sizeZ;
	fread(&sizeX, 4, 1, f);
	fread(&sizeY, 4, 1, f);
	fread(&sizeZ, 4, 1, f);

	uint32_t palette[256];
	fread(palette, 4, 256, f);

	uint8_t vx, vy, vz, colorIndex;
	while (fread(&vx, 1, 1, f) == 1)
	{
		fread(&vy, 1, 1, f);
		fread(&vz, 1, 1, f);
		fread(&colorIndex, 1, 1, f);

		uint32_t rgba = palette[colorIndex];
		uint8_t r = (rgba >> 0) & 0xff;
		uint8_t g = (rgba >> 8) & 0xff;
		uint8_t b = (rgba >> 16) & 0xff;
		uint color = (r << 16) | (g << 8) | b;
		if (color == 0) color = 1;

		Set(vx, vz, vy, color);
	}
	fclose(f);
}

void Voxel::AddSplineSegment(float3 p0, float3 p1, float3 p2, float3 p3, float duration)
{
	float alpha = 0.5f;
	float tension = 0.0f;

	float t01 = pow(length(p1 - p0), alpha);
	float t12 = pow(length(p2 - p1), alpha);
	float t23 = pow(length(p3 - p2), alpha);

	float3 m1 = (1.0f - tension) *
		(p2 - p1 + t12 * ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)));
	float3 m2 = (1.0f - tension) *
		(p2 - p1 + t12 * ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)));

	SEGMENT segment;
	segment.a = 2.0f * (p1 - p2) + m1 + m2;
	segment.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
	segment.c = m1;
	segment.d = p1;
	segment.duration = duration;
	splineSegments.push_back(segment);
}

float3 Voxel::EvaluateSpline(float t)
{
	for (auto& segment : splineSegments)
	{
		if (t <= segment.duration)
		{
			float progress = t / segment.duration;
			return segment.a * progress * progress * progress + segment.b * progress * progress + segment.c * progress + segment.d;
		}

		t -= segment.duration;
	}
	return splineSegments.back().d;
}

void Voxel::UpdateSpline(float deltaTime)
{
	
	if (splineSegments.empty())
	{
		return;
	}
	splineTime += deltaTime;

	float totalDuration = 0;
	for (auto& segment : splineSegments)
	{
		totalDuration += segment.duration;
	}
	if (splineTime > totalDuration)
	{
		splineTime -= totalDuration;
	}

	float3 pos = EvaluateSpline(splineTime);
	float scale = static_cast<float>(worldSize) / 512.0f;
	SetTransform(mat4::Translate(pos) * mat4::Scale(float3(scale)));
}

void Voxel::UpdateBrick(uint bx, uint by, uint bz)
{
	uint occupied = 0;
	const uint vx = bx * BRICKSIZE;
	const uint vy = by * BRICKSIZE;
	const uint vz = bz * BRICKSIZE;

	for (uint lz = 0; lz < BRICKSIZE && !occupied; lz++)
	{
		for (uint ly = 0; ly < BRICKSIZE && !occupied; ly++)
		{
			for (uint lx = 0; lx < BRICKSIZE && !occupied; lx++)
			{
				occupied |= grid[(vx + lx) + (vy + ly) * worldSize + (vz + lz) * worldSize2];
				if (occupied) //exit if there is a voxel in the brick
				{
					break;
				}
			}
		}
	}
	brickGrid[bx + by * brickGridSize + bz * brickGridSize2] = occupied ? 1 : 0;
}


void Voxel::Intersect(Ray& ray)
{
	Ray localRay = ray;
	localRay.O = TransformPosition(ray.O, invertedTransform);
	localRay.D = TransformVector(ray.D, invertedTransform);
	localRay.rD = float3(1.0f / localRay.D.x, 1.0f / localRay.D.y, 1.0f / localRay.D.z);
	localRay.Dsign = float3(localRay.D.x < 0 ? 1.0f : 0.0f, localRay.D.y < 0 ? 1.0f : 0.0f, localRay.D.z < 0 ? 1.0f : 0.0f);

	// setup Amanatides & Woo grid traversal
	DDAState s;
	if (!Setup3DDDA(localRay, s))
	{
		return;
	}

	uint cell, lastCell = 0, axis = localRay.axis;

	//bool voxelHit = false;
	//float voxelT = 1e34f;
	//uint voxelHitMaterial = 0;

	if (localRay.inside)
	{
		// start stepping until we find an empty voxel
		while (1)
		{
			cell = grid[s.X + s.Y * worldSize + s.Z * worldSize2];
			if (!cell) break;
			lastCell = cell;
			if (s.tmax.x < s.tmax.y)
			{
				if (s.tmax.x < s.tmax.z) { s.t = s.tmax.x, s.X += s.step.x, axis = 0; if (s.X >= worldSize) break; s.tmax.x += s.tdelta.x; }
				else { s.t = s.tmax.z, s.Z += s.step.z, axis = 2; if (s.Z >= worldSize) break; s.tmax.z += s.tdelta.z; }
			}
			else
			{
				if (s.tmax.y < s.tmax.z) { s.t = s.tmax.y, s.Y += s.step.y, axis = 1; if (s.Y >= worldSize) break; s.tmax.y += s.tdelta.y; }
				else { s.t = s.tmax.z, s.Z += s.step.z, axis = 2; if (s.Z >= worldSize) break; s.tmax.z += s.tdelta.z; }
			}
		}
		//voxelHit = lastCell != 0;
		//voxelT = s.t;
		ray.voxel = lastCell; // we store the voxel we just left
		ray.t = s.t;
		ray.axis = axis;
		ray.inside = localRay.inside;
		return;
	}

	DDAState brickDDA;
	brickDDA.step = s.step;
	brickDDA.tdelta = s.tdelta * static_cast<float>(BRICKSIZE);
	brickDDA.t = s.t;

	const float brickCell = 1.0f / brickGridSize;
	const float3 posInBrickGrid = static_cast<float>(brickGridSize) * (localRay.O + (s.t + EPSILON) * localRay.D);
	const float3 brickGridPlanes = (ceilf(posInBrickGrid) - localRay.Dsign) * brickCell;
	const int3 BP = clamp(static_cast<int3>(posInBrickGrid), 0, brickGridSize - 1);
	brickDDA.X = BP.x;
	brickDDA.Y = BP.y;
	brickDDA.Z = BP.z;
	brickDDA.tmax = (brickGridPlanes - localRay.O) * localRay.rD;

	float entryT = brickDDA.t;
	const float cellSize = 1.0f / worldSize;
	bool finishedTraversal = false;

	do
	{
		//if (sphereHit && entryT > sphereT)
		//{
		//	break;
		//}

		if (brickGrid[brickDDA.X + brickDDA.Y * brickGridSize + brickDDA.Z * brickGridSize2])
		{
			const float3 brickEntryPos = localRay.O + (entryT + EPSILON) * localRay.D;

			const uint brickMinX = brickDDA.X * BRICKSIZE;
			const uint brickMaxX = min((brickDDA.X + 1) * BRICKSIZE, static_cast<uint>(worldSize));
			const uint brickMinY = brickDDA.Y * BRICKSIZE;
			const uint brickMaxY = min((brickDDA.Y + 1) * BRICKSIZE, static_cast<uint>(worldSize));
			const uint brickMinZ = brickDDA.Z * BRICKSIZE;
			const uint brickMaxZ = min((brickDDA.Z + 1) * BRICKSIZE, static_cast<uint>(worldSize));

			DDAState innerBrickDDA;
			innerBrickDDA.step = s.step;
			innerBrickDDA.tdelta = s.tdelta;
			innerBrickDDA.t = entryT;

			innerBrickDDA.X = clamp(static_cast<uint>(brickEntryPos.x * worldSize), brickMinX, brickMaxX - 1);
			innerBrickDDA.Y = clamp(static_cast<uint>(brickEntryPos.y * worldSize), brickMinY, brickMaxY - 1);
			innerBrickDDA.Z = clamp(static_cast<uint>(brickEntryPos.z * worldSize), brickMinZ, brickMaxZ - 1);

			if (s.step.x > 0)
			{
				innerBrickDDA.tmax.x = ((innerBrickDDA.X + 1.0f) * cellSize - localRay.O.x) * localRay.rD.x;
			}
			else
			{
				innerBrickDDA.tmax.x = ((innerBrickDDA.X) * cellSize - localRay.O.x) * localRay.rD.x;
			}
			if (s.step.y > 0)
			{
				innerBrickDDA.tmax.y = ((innerBrickDDA.Y + 1.0f) * cellSize - localRay.O.y) * localRay.rD.y;
			}
			else
			{
				innerBrickDDA.tmax.y = ((innerBrickDDA.Y) * cellSize - localRay.O.y) * localRay.rD.y;
			}
			if (s.step.z > 0)
			{
				innerBrickDDA.tmax.z = ((innerBrickDDA.Z + 1.0f) * cellSize - localRay.O.z) * localRay.rD.z;
			}
			else
			{
				innerBrickDDA.tmax.z = ((innerBrickDDA.Z) * cellSize - localRay.O.z) * localRay.rD.z;
			}

			do
			{
				//if (sphereHit && innerBrickDDA.t > sphereT)
				//{
				//	finishedTraversal = true;
				//	break;
				//}
				cell = grid[innerBrickDDA.X + innerBrickDDA.Y * worldSize + innerBrickDDA.Z * worldSize2];

				if (cell)
				{
					localRay.voxel = cell;
					localRay.t = innerBrickDDA.t;
					localRay.axis = axis;
					finishedTraversal = true;
					break;
				}
				if (innerBrickDDA.tmax.x < innerBrickDDA.tmax.y)
				{
					if (innerBrickDDA.tmax.x < innerBrickDDA.tmax.z)
					{
						innerBrickDDA.t = innerBrickDDA.tmax.x;
						innerBrickDDA.X += innerBrickDDA.step.x;
						axis = 0;
						if (innerBrickDDA.X >= worldSize)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.x += innerBrickDDA.tdelta.x;
					}
					else
					{
						innerBrickDDA.t = innerBrickDDA.tmax.z;
						innerBrickDDA.Z += innerBrickDDA.step.z;
						axis = 2;
						if (innerBrickDDA.Z >= worldSize)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.z += innerBrickDDA.tdelta.z;
					}
				}
				else
				{
					if (innerBrickDDA.tmax.y < innerBrickDDA.tmax.z)
					{
						innerBrickDDA.t = innerBrickDDA.tmax.y;
						innerBrickDDA.Y += innerBrickDDA.step.y;
						axis = 1;
						if (innerBrickDDA.Y >= worldSize)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.y += innerBrickDDA.tdelta.y;
					}
					else
					{
						innerBrickDDA.t = innerBrickDDA.tmax.z;
						innerBrickDDA.Z += innerBrickDDA.step.z;
						axis = 2;
						if (innerBrickDDA.Z >= worldSize)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.z += innerBrickDDA.tdelta.z;
					}
				}

			} while (!finishedTraversal &&
				innerBrickDDA.t < ray.t &&
				innerBrickDDA.X >= brickMinX && innerBrickDDA.X < brickMaxX &&
				innerBrickDDA.Y >= brickMinY && innerBrickDDA.Y < brickMaxY &&
				innerBrickDDA.Z >= brickMinZ && innerBrickDDA.Z < brickMaxZ);
			if (finishedTraversal)
			{
				break;
			}
		}

		if (brickDDA.tmax.x < brickDDA.tmax.y)
		{
			if (brickDDA.tmax.x < brickDDA.tmax.z)
			{
				entryT = brickDDA.tmax.x;
				brickDDA.t = brickDDA.tmax.x;
				brickDDA.X += brickDDA.step.x;
				axis = 0;
				if (brickDDA.X >= brickGridSize)
				{
					break;
				}

				brickDDA.tmax.x += brickDDA.tdelta.x;
			}
			else
			{
				entryT = brickDDA.tmax.z;
				brickDDA.t = brickDDA.tmax.z;
				brickDDA.Z += brickDDA.step.z;
				axis = 2;
				if (brickDDA.Z >= brickGridSize)
				{
					break;
				}

				brickDDA.tmax.z += brickDDA.tdelta.z;
			}
		}
		else
		{
			if (brickDDA.tmax.y < brickDDA.tmax.z)
			{
				entryT = brickDDA.tmax.y;
				brickDDA.t = brickDDA.tmax.y;
				brickDDA.Y += brickDDA.step.y;
				axis = 1;
				if (brickDDA.Y >= brickGridSize)
				{
					break;
				}

				brickDDA.tmax.y += brickDDA.tdelta.y;
			}
			else
			{
				entryT = brickDDA.tmax.z;
				brickDDA.t = brickDDA.tmax.z;
				brickDDA.Z += brickDDA.step.z;
				axis = 2;
				if (brickDDA.Z >= brickGridSize)
				{
					break;
				}

				brickDDA.tmax.z += brickDDA.tdelta.z;
			}
		}

	} while (!finishedTraversal);

	if (localRay.voxel != 0 && localRay.t < ray.t)
	{
		ray.t = localRay.t;
		ray.voxel = localRay.voxel;
		ray.axis = localRay.axis;
		ray.inside = localRay.inside;

		const float3 localSign = localRay.Dsign * 2.0f - 1.0f;
		float3 localNormal = float3(localRay.axis == 0 ? localSign.x : 0, localRay.axis == 1 ? localSign.y : 0, localRay.axis == 2 ? localSign.z : 0);

		ray.N = normalize(TransformVector(localNormal, invertedTransform.Transposed()));
	}
}

bool Voxel::IsOccluded(Ray& ray)
{
	Ray localRay = ray;
	localRay.O = TransformPosition(ray.O, invertedTransform);
	localRay.D = TransformVector(ray.D, invertedTransform);
	localRay.rD = float3(1.0f / localRay.D.x, 1.0f / localRay.D.y, 1.0f / localRay.D.z);
	localRay.Dsign = float3(localRay.D.x < 0 ? 1.0f : 0.0f, localRay.D.y < 0 ? 1.0f : 0.0f, localRay.D.z < 0 ? 1.0f : 0.0f);

	DDAState brickDDA;

	// setup Amanatides & Woo grid traversal
	DDAState s;

	if (!Setup3DDDA(localRay, s)) return false;

	uint cell = 0;

	brickDDA.step = s.step;
	brickDDA.tdelta = s.tdelta * static_cast<float>(BRICKSIZE);
	brickDDA.t = s.t;

	const float brickCell = 1.0f / brickGridSize;
	const float3 posInBrickGrid = static_cast<float>(brickGridSize) * (localRay.O + (s.t + EPSILON) * localRay.D);
	const float3 brickGridPlanes = (ceilf(posInBrickGrid) - localRay.Dsign) * brickCell;
	const int3 BP = clamp(static_cast<int3>(posInBrickGrid), 0, brickGridSize - 1);
	brickDDA.X = BP.x;
	brickDDA.Y = BP.y;
	brickDDA.Z = BP.z;
	brickDDA.tmax = (brickGridPlanes - localRay.O) * localRay.rD;

	float entryT = brickDDA.t;
	const float cellSize = 1.0f / worldSize;
	bool finishedTraversal = false;

	// start stepping
	do
	{
		//if (sphereHit && entryT > sphereT)
		//{
		//	break;
		//}

		if (brickGrid[brickDDA.X + brickDDA.Y * brickGridSize + brickDDA.Z * brickGridSize2])
		{
			const float3 brickEntryPos = localRay.O + (entryT + EPSILON) * localRay.D;

			const uint brickMinX = brickDDA.X * BRICKSIZE;
			const uint brickMaxX = min((brickDDA.X + 1) * BRICKSIZE, static_cast<uint>(worldSize));
			const uint brickMinY = brickDDA.Y * BRICKSIZE;
			const uint brickMaxY = min((brickDDA.Y + 1) * BRICKSIZE, static_cast<uint>(worldSize));
			const uint brickMinZ = brickDDA.Z * BRICKSIZE;
			const uint brickMaxZ = min((brickDDA.Z + 1) * BRICKSIZE, static_cast<uint>(worldSize));

			DDAState innerBrickDDA;
			innerBrickDDA.step = s.step;
			innerBrickDDA.tdelta = s.tdelta;
			innerBrickDDA.t = entryT;

			innerBrickDDA.X = clamp(static_cast<uint>(brickEntryPos.x * worldSize), brickMinX, brickMaxX - 1);
			innerBrickDDA.Y = clamp(static_cast<uint>(brickEntryPos.y * worldSize), brickMinY, brickMaxY - 1);
			innerBrickDDA.Z = clamp(static_cast<uint>(brickEntryPos.z * worldSize), brickMinZ, brickMaxZ - 1);

			if (s.step.x > 0)
			{
				innerBrickDDA.tmax.x = ((innerBrickDDA.X + 1.0f) * cellSize - localRay.O.x) * localRay.rD.x;
			}
			else
			{
				innerBrickDDA.tmax.x = ((innerBrickDDA.X) * cellSize - localRay.O.x) * localRay.rD.x;
			}
			if (s.step.y > 0)
			{
				innerBrickDDA.tmax.y = ((innerBrickDDA.Y + 1.0f) * cellSize - localRay.O.y) * localRay.rD.y;
			}
			else
			{
				innerBrickDDA.tmax.y = ((innerBrickDDA.Y) * cellSize - localRay.O.y) * localRay.rD.y;
			}
			if (s.step.z > 0)
			{
				innerBrickDDA.tmax.z = ((innerBrickDDA.Z + 1.0f) * cellSize - localRay.O.z) * localRay.rD.z;
			}
			else
			{
				innerBrickDDA.tmax.z = ((innerBrickDDA.Z) * cellSize - localRay.O.z) * localRay.rD.z;
			}

			do
			{
				//if (sphereHit && innerBrickDDA.t > sphereT)
				//{
				//	finishedTraversal = true;
				//	break;
				//}
				cell = grid[innerBrickDDA.X + innerBrickDDA.Y * worldSize + innerBrickDDA.Z * worldSize2];

				if (cell)
				{
					return true;
				}
				if (innerBrickDDA.tmax.x < innerBrickDDA.tmax.y)
				{
					if (innerBrickDDA.tmax.x < innerBrickDDA.tmax.z)
					{
						innerBrickDDA.t = innerBrickDDA.tmax.x;
						innerBrickDDA.X += innerBrickDDA.step.x;
						if (innerBrickDDA.X >= worldSize)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.x += innerBrickDDA.tdelta.x;
					}
					else
					{
						innerBrickDDA.t = innerBrickDDA.tmax.z;
						innerBrickDDA.Z += innerBrickDDA.step.z;
						if (innerBrickDDA.Z >= worldSize)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.z += innerBrickDDA.tdelta.z;
					}
				}
				else
				{
					if (innerBrickDDA.tmax.y < innerBrickDDA.tmax.z)
					{
						innerBrickDDA.t = innerBrickDDA.tmax.y;
						innerBrickDDA.Y += innerBrickDDA.step.y;
						if (innerBrickDDA.Y >= worldSize)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.y += innerBrickDDA.tdelta.y;
					}
					else
					{
						innerBrickDDA.t = innerBrickDDA.tmax.z;
						innerBrickDDA.Z += innerBrickDDA.step.z;
						if (innerBrickDDA.Z >= worldSize)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.z += innerBrickDDA.tdelta.z;
					}
				}

			} while (!finishedTraversal &&
				innerBrickDDA.t < localRay.t &&
				innerBrickDDA.X >= brickMinX && innerBrickDDA.X < brickMaxX &&
				innerBrickDDA.Y >= brickMinY && innerBrickDDA.Y < brickMaxY &&
				innerBrickDDA.Z >= brickMinZ && innerBrickDDA.Z < brickMaxZ);
			if (finishedTraversal)
			{
				break;
			}
		}

		if (brickDDA.tmax.x < brickDDA.tmax.y)
		{
			if (brickDDA.tmax.x < brickDDA.tmax.z)
			{
				entryT = brickDDA.tmax.x;
				brickDDA.t = brickDDA.tmax.x;
				brickDDA.X += brickDDA.step.x;
				if (brickDDA.X >= brickGridSize)
				{
					break;
				}

				brickDDA.tmax.x += brickDDA.tdelta.x;
			}
			else
			{
				entryT = brickDDA.tmax.z;
				brickDDA.t = brickDDA.tmax.z;
				brickDDA.Z += brickDDA.step.z;
				if (brickDDA.Z >= brickGridSize)
				{
					break;
				}

				brickDDA.tmax.z += brickDDA.tdelta.z;
			}
		}
		else
		{
			if (brickDDA.tmax.y < brickDDA.tmax.z)
			{
				entryT = brickDDA.tmax.y;
				brickDDA.t = brickDDA.tmax.y;
				brickDDA.Y += brickDDA.step.y;
				if (brickDDA.Y >= brickGridSize)
				{
					break;
				}

				brickDDA.tmax.y += brickDDA.tdelta.y;
			}
			else
			{
				entryT = brickDDA.tmax.z;
				brickDDA.t = brickDDA.tmax.z;
				brickDDA.Z += brickDDA.step.z;
				if (brickDDA.Z >= brickGridSize)
				{
					break;
				}

				brickDDA.tmax.z += brickDDA.tdelta.z;
			}
		}

	} while (!finishedTraversal && brickDDA.t < localRay.t);

	return false;
}

