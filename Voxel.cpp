#include "template.h"
#include "Voxel.h"

Voxel::Voxel()
{
	// allocate room for the world
	grid = (uint*)MALLOC64(WORLDSIZE3 * sizeof(uint));
	memset(grid, 0, WORLDSIZE3 * sizeof(uint));
	brickGrid = 0;
	brickGrid = (uint8_t*)MALLOC64(BRICKGRID3 * sizeof(uint8_t));
}

Voxel::~Voxel()
{
	FREE64(grid);
	FREE64(brickGrid);
}

void Voxel::Set(const uint x, const uint y, const uint z, const uint v)
{
	const uint old = grid[x + y * WORLDSIZE + z * WORLDSIZE2];
	grid[x + y * WORLDSIZE + z * WORLDSIZE2] = v;

	if (!brickGrid || old == v)
	{
		return;
	}
	const uint brickX = x / BRICKSIZE;
	const uint brickY = y / BRICKSIZE;
	const uint brickZ = z / BRICKSIZE;

	if (v != 0)
	{
		brickGrid[brickX + brickY * BRICKGRID + brickZ * BRICKGRID2] = 1;
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

	aabbMin = float3(1e34f);
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
	static const float cellSize = 1.0f / WORLDSIZE;
	state.step = make_int3(1.0f - ray.Dsign * 2.0f);
	const float3 posInGrid = (float)WORLDSIZE * (ray.O + (state.t + 0.00005f) * ray.D);
	const float3 gridPlanes = (ceilf(posInGrid) - ray.Dsign) * cellSize;
	const int3 P = clamp(make_int3(posInGrid), 0, WORLDSIZE - 1);
	state.X = P.x, state.Y = P.y, state.Z = P.z;
	state.tdelta = cellSize * float3(state.step) * ray.rD;
	state.tmax = (gridPlanes - ray.O) * ray.rD;
	// detect rays that start inside a voxel
	uint cell = grid[P.x + P.y * WORLDSIZE + P.z * WORLDSIZE2];
	ray.inside = cell != 0 && startedInGrid;
	// proceed with traversal
	return true;
}

void Voxel::BuildBrickGrid()
{
	memset(brickGrid, 0, BRICKGRID3 * sizeof(uint8_t)); // makes sure that the whole grid starts empty
	for (uint bz = 0; bz < BRICKGRID; bz++)
	{
		for (uint by = 0; by < BRICKGRID; by++)
		{
			for (uint bx = 0; bx < BRICKGRID; bx++)
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
							occupied |= grid[(vx + lx) + (vy + ly) * WORLDSIZE + (vz + lz) * WORLDSIZE2];
						}
					}
				}
				// if occupied is 1, at least one voxel was solid inside the brick
				brickGrid[bx + by * BRICKGRID + bz * BRICKGRID2] = occupied ? 1 : 0;
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
				occupied |= grid[(vx + lx) + (vy + ly) * WORLDSIZE + (vz + lz) * WORLDSIZE2];
				if (occupied) //exit if there is a voxel in the brick
				{
					break;
				}
			}
		}
	}
	brickGrid[bx + by * BRICKGRID + bz * BRICKGRID2] = occupied ? 1 : 0;
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

	uint cell, lastCell = 0, axis = 0;

	//bool voxelHit = false;
	//float voxelT = 1e34f;
	//uint voxelHitMaterial = 0;

	if (localRay.inside)
	{
		// start stepping until we find an empty voxel
		while (1)
		{
			cell = grid[s.X + s.Y * WORLDSIZE + s.Z * WORLDSIZE2];
			if (!cell) break;
			lastCell = cell;
			if (s.tmax.x < s.tmax.y)
			{
				if (s.tmax.x < s.tmax.z) { s.t = s.tmax.x, s.X += s.step.x, axis = 0; if (s.X >= WORLDSIZE) break; s.tmax.x += s.tdelta.x; }
				else { s.t = s.tmax.z, s.Z += s.step.z, axis = 2; if (s.Z >= WORLDSIZE) break; s.tmax.z += s.tdelta.z; }
			}
			else
			{
				if (s.tmax.y < s.tmax.z) { s.t = s.tmax.y, s.Y += s.step.y, axis = 1; if (s.Y >= WORLDSIZE) break; s.tmax.y += s.tdelta.y; }
				else { s.t = s.tmax.z, s.Z += s.step.z, axis = 2; if (s.Z >= WORLDSIZE) break; s.tmax.z += s.tdelta.z; }
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

	const float brickCell = 1.0f / BRICKGRID;
	const float3 posInBrickGrid = static_cast<float>(BRICKGRID) * (localRay.O + (s.t + EPSILON) * localRay.D);
	const float3 brickGridPlanes = (ceilf(posInBrickGrid) - localRay.Dsign) * brickCell;
	const int3 BP = clamp(static_cast<int3>(posInBrickGrid), 0, BRICKGRID - 1);
	brickDDA.X = BP.x;
	brickDDA.Y = BP.y;
	brickDDA.Z = BP.z;
	brickDDA.tmax = (brickGridPlanes - localRay.O) * localRay.rD;

	float entryT = brickDDA.t;
	const float cellSize = 1.0f / WORLDSIZE;
	bool finishedTraversal = false;

	do
	{
		//if (sphereHit && entryT > sphereT)
		//{
		//	break;
		//}

		if (brickGrid[brickDDA.X + brickDDA.Y * BRICKGRID + brickDDA.Z * BRICKGRID2])
		{
			const float3 brickEntryPos = localRay.O + (entryT + EPSILON) * localRay.D;

			const uint brickMinX = brickDDA.X * BRICKSIZE;
			const uint brickMaxX = min((brickDDA.X + 1) * BRICKSIZE, static_cast<uint>(WORLDSIZE));
			const uint brickMinY = brickDDA.Y * BRICKSIZE;
			const uint brickMaxY = min((brickDDA.Y + 1) * BRICKSIZE, static_cast<uint>(WORLDSIZE));
			const uint brickMinZ = brickDDA.Z * BRICKSIZE;
			const uint brickMaxZ = min((brickDDA.Z + 1) * BRICKSIZE, static_cast<uint>(WORLDSIZE));

			DDAState innerBrickDDA;
			innerBrickDDA.step = s.step;
			innerBrickDDA.tdelta = s.tdelta;
			innerBrickDDA.t = entryT;

			innerBrickDDA.X = clamp(static_cast<uint>(brickEntryPos.x * WORLDSIZE), brickMinX, brickMaxX - 1);
			innerBrickDDA.Y = clamp(static_cast<uint>(brickEntryPos.y * WORLDSIZE), brickMinY, brickMaxY - 1);
			innerBrickDDA.Z = clamp(static_cast<uint>(brickEntryPos.z * WORLDSIZE), brickMinZ, brickMaxZ - 1);

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
				cell = grid[innerBrickDDA.X + innerBrickDDA.Y * WORLDSIZE + innerBrickDDA.Z * WORLDSIZE2];

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
						if (innerBrickDDA.X >= WORLDSIZE)
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
						if (innerBrickDDA.Z >= WORLDSIZE)
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
						if (innerBrickDDA.Y >= WORLDSIZE)
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
						if (innerBrickDDA.Z >= WORLDSIZE)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.z += innerBrickDDA.tdelta.z;
					}
				}

			} while (!finishedTraversal &&
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
				if (brickDDA.X >= BRICKGRID)
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
				if (brickDDA.Z >= BRICKGRID)
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
				if (brickDDA.Y >= BRICKGRID)
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
				if (brickDDA.Z >= BRICKGRID)
				{
					break;
				}

				brickDDA.tmax.z += brickDDA.tdelta.z;
			}
		}

	} while (!finishedTraversal);

	ray.t = localRay.t;
	ray.voxel = localRay.voxel;
	ray.axis = localRay.axis;
	ray.inside = localRay.inside;
	
	const float3 localSign = localRay.Dsign * 2.0f - 1.0f;
	float3 localNormal = float3(localRay.axis == 0 ? localSign.x : 0, localRay.axis == 1 ? localSign.y : 0, localRay.axis == 2 ? localSign.z : 0);

	ray.N = normalize(TransformVector(localNormal, transform));
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

	const float brickCell = 1.0f / BRICKGRID;
	const float3 posInBrickGrid = static_cast<float>(BRICKGRID) * (localRay.O + (s.t + EPSILON) * localRay.D);
	const float3 brickGridPlanes = (ceilf(posInBrickGrid) - localRay.Dsign) * brickCell;
	const int3 BP = clamp(static_cast<int3>(posInBrickGrid), 0, BRICKGRID - 1);
	brickDDA.X = BP.x;
	brickDDA.Y = BP.y;
	brickDDA.Z = BP.z;
	brickDDA.tmax = (brickGridPlanes - localRay.O) * localRay.rD;

	float entryT = brickDDA.t;
	const float cellSize = 1.0f / WORLDSIZE;
	bool finishedTraversal = false;

	// start stepping
	do
	{
		//if (sphereHit && entryT > sphereT)
		//{
		//	break;
		//}

		if (brickGrid[brickDDA.X + brickDDA.Y * BRICKGRID + brickDDA.Z * BRICKGRID2])
		{
			const float3 brickEntryPos = localRay.O + (entryT + EPSILON) * localRay.D;

			const uint brickMinX = brickDDA.X * BRICKSIZE;
			const uint brickMaxX = min((brickDDA.X + 1) * BRICKSIZE, static_cast<uint>(WORLDSIZE));
			const uint brickMinY = brickDDA.Y * BRICKSIZE;
			const uint brickMaxY = min((brickDDA.Y + 1) * BRICKSIZE, static_cast<uint>(WORLDSIZE));
			const uint brickMinZ = brickDDA.Z * BRICKSIZE;
			const uint brickMaxZ = min((brickDDA.Z + 1) * BRICKSIZE, static_cast<uint>(WORLDSIZE));

			DDAState innerBrickDDA;
			innerBrickDDA.step = s.step;
			innerBrickDDA.tdelta = s.tdelta;
			innerBrickDDA.t = entryT;

			innerBrickDDA.X = clamp(static_cast<uint>(brickEntryPos.x * WORLDSIZE), brickMinX, brickMaxX - 1);
			innerBrickDDA.Y = clamp(static_cast<uint>(brickEntryPos.y * WORLDSIZE), brickMinY, brickMaxY - 1);
			innerBrickDDA.Z = clamp(static_cast<uint>(brickEntryPos.z * WORLDSIZE), brickMinZ, brickMaxZ - 1);

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
				cell = grid[innerBrickDDA.X + innerBrickDDA.Y * WORLDSIZE + innerBrickDDA.Z * WORLDSIZE2];

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
						if (innerBrickDDA.X >= WORLDSIZE)
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
						if (innerBrickDDA.Z >= WORLDSIZE)
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
						if (innerBrickDDA.Y >= WORLDSIZE)
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
						if (innerBrickDDA.Z >= WORLDSIZE)
						{
							finishedTraversal = true;
							break;
						}

						innerBrickDDA.tmax.z += innerBrickDDA.tdelta.z;
					}
				}

			} while (!finishedTraversal &&
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
				if (brickDDA.X >= BRICKGRID)
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
				if (brickDDA.Z >= BRICKGRID)
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
				if (brickDDA.Y >= BRICKGRID)
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
				if (brickDDA.Z >= BRICKGRID)
				{
					break;
				}

				brickDDA.tmax.z += brickDDA.tdelta.z;
			}
		}

	} while (!finishedTraversal && brickDDA.t < localRay.t);

	return false;
}

