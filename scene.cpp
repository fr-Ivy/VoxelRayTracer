#include "template.h"

#include <set>

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

inline float IntersectSphere(Ray& ray, const Sphere& sphere)
{
	float3 c = sphere.center - ray.O;
	float t = dot(c, ray.D);
	float3 q = c - t * ray.D;
	float p2 = dot(q, q);
	float radius2 = pow2f(sphere.radius);
	if (p2 > radius2)
	{
		return 1e34f;
	}

	t -= sqrt(radius2 - p2);
	if ((t > 0))
	{
		return t;
	}
	return 1e34f;
}

Scene::Scene()
{
	// allocate room for the world
	grid = (uint*)MALLOC64(WORLDSIZE3 * sizeof(uint));
	memset(grid, 0, WORLDSIZE3 * sizeof(uint));

	//sphereGrid = static_cast<uint*>(MALLOC64(WORLDSIZE3 * sizeof(uint)));
	//memset(sphereGrid, 0, WORLDSIZE3 * sizeof(uint));

	//std::vector<Sphere> spheres;

	switch (3)
	{
	case 1:
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
	case 2:
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
							Set(x, y, z, 0xaaffaa);
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
	case 3:
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
	case 4:
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
	case 5:
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
	case 6:
	{
		//-----------------------AI GENERATED SCENE----------------------------------
		const int sphereCount2 = 1000;
		for (int i = 0; i < sphereCount2; i++)
		{
			// random center in voxel space
			float x = rand() % 512;
			float y = rand() % 512;
			float z = rand() % 512;

			// random radius between 5 and 20 voxels
			float radiusVox = 5 + (rand() % 16);

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
	default:
		break;
	}
}

void Scene::Set(const uint x, const uint y, const uint z, const uint v)
{
	grid[x + y * WORLDSIZE + z * WORLDSIZE2] = v;
}

void Scene::SetSphere(float3 center, float radius, uint material)
{
	spheres.push_back({ center / WORLDSIZE, radius / WORLDSIZE, material });
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

void Scene::FindNearest(Ray& ray) const
{
	// nudge origin
	ray.O += EPSILON * ray.D;

	float bestTSphere = 1e34f;
	int bestSphere = -1;

	for (int i = 0; i < static_cast<int>(spheres.size()); i++)
	{
		float distance = IntersectSphere(ray, spheres[i]);
		if (distance < bestTSphere)
		{
			bestTSphere = distance;
			bestSphere = i;
		}
	}

	// setup Amanatides & Woo grid traversal
	DDAState s;
	if (!Setup3DDDA(ray, s)) return;
	uint cell, lastCell = 0, axis = ray.axis;

	float sphereHit = 1e34f;
	uint sphereHitMaterial = 0;
	uint sphereHitAxis = 0;
	uint sphereAxis = 0;
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
		ray.voxel = lastCell; // we store the voxel we just left
	}
	else
	{
		// start stepping until we find a filled voxel
		while (1)
		{
			cell = grid[s.X + s.Y * WORLDSIZE + s.Z * WORLDSIZE2];
			if (cell) break; else if (s.tmax.x < s.tmax.y)
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
		ray.voxel = cell;
		voxelHitMaterial = ray.voxel;
	}

	if (bestSphere >= 0 && bestTSphere < s.t)
	{
		ray.t = bestTSphere;
		ray.voxel = spheres[bestSphere].material;
		ray.hitSphere = true;

		float3 hitPos = ray.O + ray.t * ray.D;
		ray.N = normalize(hitPos - spheres[bestSphere].center);

		return;
	}

	ray.t = s.t;
	ray.axis = axis;
	ray.voxel = voxelHitMaterial;
	ray.hitSphere = false;
}

bool Scene::IsOccluded(Ray& ray) const
{
	// nudge origin
	ray.O += EPSILON * ray.D;
	ray.t -= EPSILON * 2.0f;

	//setup shadows for the spheres
	for (int i = 0; i < static_cast<int>(spheres.size()); i++)
	{
		float distance = IntersectSphere(ray, spheres[i]);
		if (distance < ray.t)
		{
			return true;
		}
	}

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