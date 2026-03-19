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
	// setup Amanatides & Woo grid traversal
	DDAState s;
	if (!Setup3DDDA(ray, s))
	{
		return;
	}
	uint cell, lastCell = 0, axis = ray.axis;



	//bool voxelHit = false;
	//float voxelT = 1e34f;
	//uint voxelHitMaterial = 0;

	if (ray.inside)
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
		//voxelHitMaterial = lastCell; // we store the voxel we just left
		ray.t = s.t;
		ray.axis = axis;
		return;
	}

	DDAState brickDDA;
	brickDDA.step = s.step;
	brickDDA.tdelta = s.tdelta * static_cast<float>(BRICKSIZE);
	brickDDA.t = s.t;

	const float brickCell = 1.0f / BRICKGRID;
	const float3 posInBrickGrid = static_cast<float>(BRICKGRID) * (ray.O + (s.t + EPSILON) * ray.D);
	const float3 brickGridPlanes = (ceilf(posInBrickGrid) - ray.Dsign) * brickCell;
	const int3 BP = clamp(static_cast<int3>(posInBrickGrid), 0, BRICKGRID - 1);
	brickDDA.X = BP.x;
	brickDDA.Y = BP.y;
	brickDDA.Z = BP.z;
	brickDDA.tmax = (brickGridPlanes - ray.O) * ray.rD;

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
			const float3 brickEntryPos = ray.O + (entryT + EPSILON) * ray.D;

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
				innerBrickDDA.tmax.x = ((innerBrickDDA.X + 1.0f) * cellSize - ray.O.x) * ray.rD.x;
			}
			else
			{
				innerBrickDDA.tmax.x = ((innerBrickDDA.X) * cellSize - ray.O.x) * ray.rD.x;
			}
			if (s.step.y > 0)
			{
				innerBrickDDA.tmax.y = ((innerBrickDDA.Y + 1.0f) * cellSize - ray.O.y) * ray.rD.y;
			}
			else
			{
				innerBrickDDA.tmax.y = ((innerBrickDDA.Y) * cellSize - ray.O.y) * ray.rD.y;
			}
			if (s.step.z > 0)
			{
				innerBrickDDA.tmax.z = ((innerBrickDDA.Z + 1.0f) * cellSize - ray.O.z) * ray.rD.z;
			}
			else
			{
				innerBrickDDA.tmax.z = ((innerBrickDDA.Z) * cellSize - ray.O.z) * ray.rD.z;
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
					ray.voxel = cell;
					ray.t = innerBrickDDA.t;
					ray.axis = axis;
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

}

bool Voxel::IsOccluded(Ray& ray)
{
	DDAState brickDDA;

	// setup Amanatides & Woo grid traversal
	DDAState s;

	if (!Setup3DDDA(ray, s)) return false;

	uint cell = 0;

	brickDDA.step = s.step;
	brickDDA.tdelta = s.tdelta * static_cast<float>(BRICKSIZE);
	brickDDA.t = s.t;

	const float brickCell = 1.0f / BRICKGRID;
	const float3 posInBrickGrid = static_cast<float>(BRICKGRID) * (ray.O + (s.t + EPSILON) * ray.D);
	const float3 brickGridPlanes = (ceilf(posInBrickGrid) - ray.Dsign) * brickCell;
	const int3 BP = clamp(static_cast<int3>(posInBrickGrid), 0, BRICKGRID - 1);
	brickDDA.X = BP.x;
	brickDDA.Y = BP.y;
	brickDDA.Z = BP.z;
	brickDDA.tmax = (brickGridPlanes - ray.O) * ray.rD;

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
			const float3 brickEntryPos = ray.O + (entryT + EPSILON) * ray.D;

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
				innerBrickDDA.tmax.x = ((innerBrickDDA.X + 1.0f) * cellSize - ray.O.x) * ray.rD.x;
			}
			else
			{
				innerBrickDDA.tmax.x = ((innerBrickDDA.X) * cellSize - ray.O.x) * ray.rD.x;
			}
			if (s.step.y > 0)
			{
				innerBrickDDA.tmax.y = ((innerBrickDDA.Y + 1.0f) * cellSize - ray.O.y) * ray.rD.y;
			}
			else
			{
				innerBrickDDA.tmax.y = ((innerBrickDDA.Y) * cellSize - ray.O.y) * ray.rD.y;
			}
			if (s.step.z > 0)
			{
				innerBrickDDA.tmax.z = ((innerBrickDDA.Z + 1.0f) * cellSize - ray.O.z) * ray.rD.z;
			}
			else
			{
				innerBrickDDA.tmax.z = ((innerBrickDDA.Z) * cellSize - ray.O.z) * ray.rD.z;
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

	} while (!finishedTraversal && brickDDA.t < ray.t);

	return false;
}

