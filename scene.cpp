#include "template.h"

#include <set>

#include "bvh.h"

inline float intersect_cube(Ray& ray)
{
	// branchless slab method by Tavian
	const float tx1 = -ray.O.x * ray.rD.x, tx2 = (1 - ray.O.x) * ray.rD.x;
	float ty, tz, tmin = min(tx1, tx2), tmax = max(tx1, tx2);
	const float ty1 = -ray.O.y * ray.rD.y, ty2 = (1 - ray.O.y) * ray.rD.y;
	ty = min(ty1, ty2), tmin = max(tmin, ty), tmax = min(tmax, max(ty1, ty2));
	const float tz1 = -ray.O.z * ray.rD.z, tz2 = (1 - ray.O.z) * ray.rD.z;
	tz = min(tz1, tz2), tmin = max(tmin, tz), tmax = min(tmax, max(tz1, tz2));
	if (tmin == tz) ray.axis = 2; else if (tmin == ty) ray.axis = 1;
	return tmax >= tmin && tmin > 0 ? tmin : 1e34f;
}

inline bool point_in_cube(const float3& pos)
{
	// test if pos is inside the cube
	return pos.x >= 0 && pos.y >= 0 && pos.z >= 0 &&
		pos.x <= 1 && pos.y <= 1 && pos.z <= 1;
}


Scene::Scene()
{
	// allocate room for the world
	grid = (uint*)MALLOC64(WORLDSIZE3 * sizeof(uint));
	memset(grid, 0, WORLDSIZE3 * sizeof(uint));
	brickGrid = 0;

	//sphereGrid = static_cast<uint*>(MALLOC64(WORLDSIZE3 * sizeof(uint)));
	//memset(sphereGrid, 0, WORLDSIZE3 * sizeof(uint));

	//std::vector<Sphere> spheres;

	switch (6)
	{
	case 1:
	{
		// initialize the scene using Perlin noise, parallel over z
		for (int z = 0; z < 128; z++)
		{
			const float fz = (float)z / 128;
			for (int y = 0; y < 128; y++)
			{
				float fx = 0, fy = (float)y / 128;
				for (int x = 0; x < 128; x++, fx += 1.0f / 128)
				{
					const float n = noise3D(fx, fy, fz);
					Set(x, y, z, n > 0.09f ? 0x01020101 * y : 0);
				}
			}
		}
		break;
	}
	case 2:
	{
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 128; z++)
		{
			for (int y = 0; y < 128; y++)
			{
				for (int x = 0; x < 128; x++)
				{
					if (x < 2 || x > 125 || z > 125 || y < 2 || y > 125 /*|| z < 2*/)
					{
						//Set(x, y, z, y == 1 || x == 1 || x == 126 || y == 126 || z == 126 /*|| z == 1*/ ? 0x49999bb : 0xffffff);
						Set(x, y, z, 0xeeeeee);
					}
					else if (y > 30 && y < 50 && z > 50 && z < 70 && x > 20)
					{
						if (x < 40)
						{
							Set(x, y, z, 0xff7777 /*x == 39 || x == 21 || z == 51 || z == 69 || y == 31 || y == 49 ? 0x19999bb : 0xffffff*/);
						}
						else if (x > 55 && x < 75)
						{
							Set(x, y, z, 0x02aaffaa);
						}



						else if (x > 90 && x < 110)
						{
							Set(x, y, z, 0x7777ff);
						}
					}

					//sphere position = (65, 40, 60)
					//radius = 10
					//formula sphere = (x - a)^2 + (y - b)^2 + (z - c)^2 = r^2

					//if (pow2f(static_cast<float>(x - 65)) + pow2f(static_cast<float>(y - 40)) + pow2f(static_cast<float>(z - 60)) <= pow2f(10))
					//{
						//Set(x, y, z, 0x02aaffaa);
					//}

				}
			}
		}
		break;
	}
	case 3:
	{
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 256; z++)
		{
			for (int y = 0; y < 256; y++)
			{
				for (int x = 0; x < 256; x++)
				{
					if (x > 253 || y < 2)
					{
						Set(x, y, z, x == 254 ? 0x01eeeeee : 0xeeeeee);
					}

					if (pow2f(static_cast<float>(x - 128)) + pow2f(static_cast<float>(y - 2 - 20)) + pow2f(static_cast<float>(z - 128)) <= pow2f(20))
					{
						Set(x, y, z, 0xaaffaa);
					}

					if (x >= 150 && x < 170 && y >= 2 && y < 22 && z >= 100 && z < 120)
					{
						Set(x, y, z, 0x5ffffff);
					}


				}
			}
		}

		SetSphere(float3(200, 25, 50), 25.0f, 0x0400ff00);
		SetSphere(float3(25, 25, 25), 25.0f, 0xffffff);

		break;
	}
	case 4:
	{
		//-----------------------AI GENERATED SCENE----------------------------------
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 512; z++)
		{
			for (int y = 0; y < 512; y++)
			{
				for (int x = 0; x < 512; x++)
				{
					if (x > 509 || y < 4)
					{
						Set(x, y, z, x == 510 ? 0x04eeeeee : 0xeeeeee);
					}

					const int sphereCount = 12;
					const float radius = 10.0f;
					const float spacing = 30.0f;

					const float startX = 80.0f;
					const float centerY = radius + 4.0f;
					const float centerZ = 256.0f;

					for (int i = 0; i < sphereCount; i++)
					{
						float cx = startX + i * spacing;

						float dx = x - cx;
						float dy = y - centerY;
						float dz = z - centerZ;

						if (pow2f(dx) + pow2f(dy) + pow2f(dz) <= pow2f(radius))
						{
							uint32_t color = 0x880000 + (i * 0x000f0f);
							Set(x, y, z, color);
						}
					}
				}
			}
		}
		break;
	}
	case 5:
	{
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 512; z++)
		{
			for (int y = 0; y < 512; y++)
			{
				for (int x = 0; x < 512; x++)
				{
					if (x < 2 || x > 509 || z > 509 || y < 2 || y > 509 || z < 2)
					{

						Set(x, y, z, /*y == 1 || x == 1 || x == 510 || y == 510 || z == 510 ||*/ z == 1 ? 0x019999bb : 0xffffff);
						//Set(x, y, z, 0xeeeeee);
					}

					if (y > 30 && y < 50 && z > 50 && z < 70 && x > 20)
					{
						if (x > 55 && x < 75)
						{
							Set(x, y, z, 0x02ffffff);
						}
					}
				}
			}
		}
		break;
	}
	case 6:
	{
		//-----------------------AI GENERATED SCENE----------------------------------

		for (int z = 0; z < 512; z++)
		{
			for (int x = 0; x < 512; x++)
			{
				for (int y = 0; y < 10; y++)   // platform thickness
				{
					uint color =
						(0x00 << 24) |   // diffuse
						(180 << 16) |
						(180 << 8) |
						180;
					Set(x, y, z, color);
				}
			}
		}


		const int sphereCount2 = 1000;
		for (int i = 0; i < sphereCount2; i++)
		{
			// random center in voxel space
			float x = static_cast<float>(rand() % 512);
			float y = 50.0f + static_cast<float>(rand() % 400);
			float z = static_cast<float>(rand() % 512);

			// random radius between 5 and 20 voxels
			float radiusVox = 1.0f + static_cast<float>(rand() % 16);
			//float radiusVox = 20.0f;

			// scale to 0–1 world space
			float3 center = float3(x, y, z);
			float radius = radiusVox;

			// random NON-reflective color
			// highest byte = material type
			// 0x00 = diffuse
			uint color =
				(0x00 << 24) |                 // diffuse material
				((rand() % 256) << 16) |       // R
				((rand() % 256) << 8) |       // G
				(rand() % 256);                // B

			SetSphere(center, radius, color);
		}
		break;
	}
	case 7:
	{
		// initialize the scene using Perlin noise, parallel over z
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < WORLDSIZE; z++)
		{
			const float fz = (float)z / WORLDSIZE;
			for (int y = 0; y < WORLDSIZE; y++)
			{
				float fx = 0, fy = (float)y / WORLDSIZE;
				for (int x = 0; x < WORLDSIZE; x++, fx += 1.0f / WORLDSIZE)
				{
					const float n = noise3D(fx, fy, fz);
					uint8_t r = std::min(2 * y, 255);
					uint8_t g = std::min(1 * y, 255);
					uint8_t b = std::min(1 * y, 255);

					uint32_t c = (r << 16) | (g << 8) | b;
					Set(x, y, z, n > 0.09f ? c : 0);
				}
			}
		}
		break;
	}
	case 8:
	{
		const int stepCount = 30;
		const int stepHeight = 1;
		const int stepDepth = 10;
		const int stepWidth = 50;

		const int baseX = 200;
		const int baseZ = 200;

		uint stepColor =
			(0x00 << 24) |
			(160 << 16) |
			(160 << 8) |
			160;

		for (int i = 0; i < stepCount; i++)
		{
			int yStart = i * stepHeight;
			int zStart = baseZ + i * stepDepth;

			for (int y = yStart; y < yStart + stepHeight; y++)
				for (int z = zStart; z < zStart + stepDepth; z++)
					for (int x = baseX; x < baseX + stepWidth; x++)
						Set(x, y, z, stepColor);
		}

		float3 ballCenter;
		ballCenter.x = baseX + stepWidth / 2;
		ballCenter.y = stepCount * stepHeight + 10;
		ballCenter.z = baseZ + stepCount * stepDepth - 10;

		float ballRadius = 12.0f;

		uint ballColor =
			(0x00 << 24) |
			(255 << 16) |
			(80 << 8) |
			80;

		SetSphere(ballCenter, ballRadius, ballColor);

		uint wallColor =
			(0x00 << 24) |
			(120 << 16) |
			(120 << 8) |
			120;

		int wallY = stepCount * stepHeight;
		int wallZ = baseZ + stepCount * stepDepth;

		for (int y = wallY; y < wallY + 40; y++)
			for (int z = wallZ; z < wallZ + 5; z++)
				for (int x = baseX; x < baseX + stepWidth; x++)
					Set(x, y, z, wallColor);

		for (int y = 0; y < wallY + 40; y++)
			for (int z = baseZ; z < wallZ; z++)
				for (int x = baseX - 2; x < baseX; x++)
					Set(x, y, z, wallColor);

		for (int y = 0; y < wallY + 40; y++)
			for (int z = baseZ; z < wallZ; z++)
				for (int x = baseX + stepWidth; x < baseX + stepWidth + 2; x++)
					Set(x, y, z, wallColor);

		break;
	}
	default:
		break;
	}

	brickGrid = (uint8_t*)MALLOC64(BRICKGRID3 * sizeof(uint8_t));
	BuildBrickGrid();
	bvh = new BVH();
	bvh->BuildBVH(*this);
}

Scene::~Scene()
{
	delete bvh;
}

void Scene::Set(const uint x, const uint y, const uint z, const uint v)
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

void Scene::SetSphere(float3 center, float radius, uint material)
{
	Sphere s;
	s.center = center / WORLDSIZE;
	s.radius = radius / WORLDSIZE;
	s.material = material;
	s.velocity = float3(0);
	s.previousCenter = s.center;
	spheres.push_back(s);
}

bool Scene::Setup3DDDA(Ray& ray, DDAState& state) const
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

void Scene::BuildBrickGrid()
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

void Scene::UpdateBrick(uint bx, uint by, uint bz)
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

void Scene::FindNearest(Ray& ray, bool skipBVH) const
{
	// nudge origin
	ray.O += EPSILON * ray.D;

	if (!skipBVH)
	{
		// check for sphere intersections using the BVH
		bvh->IntersectBVH(ray, *this);
	}

	// store sphere hit information before starting the voxel traversal.
	float sphereT = ray.t;
	bool sphereHit = ray.hitSphere;
	uint sphereVoxel = ray.voxel;
	float3 sphereN = ray.N;
	
	ray.t = 1e34f;

	//float bestTSphere = 1e34f;
	//int bestSphere = -1;

	//for (int i = 0; i < static_cast<int>(spheres.size()); i++)
	//{
	//	float distance = IntersectSphere(ray, spheres[i]);
	//	if (distance < bestTSphere)
	//	{
	//		bestTSphere = distance;
	//		bestSphere = i;
	//	}
	//}



	// setup Amanatides & Woo grid traversal
	DDAState s;
	if (!Setup3DDDA(ray, s))
	{
		ray.t = sphereT;
		ray.hitSphere = sphereHit;
		ray.voxel = sphereVoxel;
		ray.N = sphereN;
		return;
	}
	uint cell, lastCell = 0, axis = ray.axis;

	//float sphereHit = 1e34f;
	//uint sphereHitMaterial = 0;
	//uint sphereHitAxis = 0;
	//uint sphereAxis = 0;

	bool voxelHit = false;
	float voxelT = 1e34f;
	uint voxelHitMaterial = 0;

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
		voxelHit = lastCell != 0;
		voxelT = s.t;
		voxelHitMaterial = lastCell; // we store the voxel we just left
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


	// check if a sphere was hit and if it's closer than a voxel hit, update the ray with sphere information.
	if (sphereHit && sphereT < ray.t)
	{
		ray.t = sphereT;
		ray.voxel = sphereVoxel;
		ray.hitSphere = true;
		ray.N = sphereN;
	}

	// if a voxel was hit and it's closer then a sphere hit, update the ray with voxel information.
	else
	{
		//ray.t = voxelT;
		//ray.axis = axis;
		//ray.voxel = voxelHitMaterial;
		ray.hitSphere = false;
	}
}


bool Scene::IsOccluded(Ray& ray) const
{
	if (!shadows)
	{
		return false;
	}
	// nudge origin
	ray.O += EPSILON * ray.D;
	ray.t -= EPSILON * 2.0f;

	ray.hitSphere = false;

	//setup shadows for the spheres

	if (!bvh->bvhNodes.empty())
	{
		bvh->IntersectBVH(ray, *this);
	}
	if (ray.hitSphere)
	{
		return true;
	}

	//for (int i = 0; i < static_cast<int>(spheres.size()); i++)
	//{
	//	float distance = IntersectSphere(ray, spheres[i]);
	//	if (distance < ray.t)
	//	{
	//		return true;
	//	}
	//}

	// setup Amanatides & Woo grid traversal
	DDAState s;
	if (!Setup3DDDA(ray, s)) return false;
	// start stepping
	while (s.t < ray.t)
	{
		const uint cell = grid[s.X + s.Y * WORLDSIZE + s.Z * WORLDSIZE2];
		if (cell) /* we hit a solid voxel */ return s.t < ray.t;
		if (s.tmax.x < s.tmax.y)
		{
			if (s.tmax.x < s.tmax.z) { if ((s.X += s.step.x) >= WORLDSIZE) return false; s.t = s.tmax.x, s.tmax.x += s.tdelta.x; }
			else { if ((s.Z += s.step.z) >= WORLDSIZE) return false; s.t = s.tmax.z, s.tmax.z += s.tdelta.z; }
		}
		else
		{
			if (s.tmax.y < s.tmax.z) { if ((s.Y += s.step.y) >= WORLDSIZE) return false; s.t = s.tmax.y, s.tmax.y += s.tdelta.y; }
			else { if ((s.Z += s.step.z) >= WORLDSIZE) return false; s.t = s.tmax.z, s.tmax.z += s.tdelta.z; }
		}
	}
	return false;
}