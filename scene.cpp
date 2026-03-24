#include "template.h"

#include <set>

#include "bvh.h"
#include "voxel.h"
#include "TLAS.h"

Scene::Scene()
{
	switch (10)
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
					voxels->Set(x, y, z, n > 0.09f ? 0x01020101 * y : 0);
				}
			}
		}
		break;
	}
	case 2:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].SetTransform(mat4::Identity());
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 128; z++)
		{
			for (int y = 0; y < 128; y++)
			{
				for (int x = 0; x < 128; x++)
				{
					if (x < 2 || x > 125 /*|| z > 125*/ || y < 2 || y > 125 /*|| z < 2*/)
					{
						//Set(x, y, z, y == 1 || x == 1 || x == 126 || y == 126 || z == 126 /*|| z == 1*/ ? 0x49999bb : 0xffffff);
						voxels[0].Set(x, y, z, 0xeeeeee);
					}
					else if (y > 30 && y < 50 && z > 50 && z < 70 && x > 20)
					{
						if (x < 40)
						{
							voxels[0].Set(x, y, z, 0xff7777 /*x == 39 || x == 21 || z == 51 || z == 69 || y == 31 || y == 49 ? 0x19999bb : 0xffffff*/);
						}
						else if (x > 55 && x < 75)
						{
							voxels[0].Set(x, y, z, 0x02aaffaa);
						}



						else if (x > 90 && x < 110)
						{
							voxels[0].Set(x, y, z, 0x7777ff);
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
		voxels[0].BuildBrickGrid();
		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 3:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)));
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 256; z++)
		{
			for (int y = 0; y < 256; y++)
			{
				for (int x = 0; x < 256; x++)
				{
					if (x > 253 || y < 2)
					{
						voxels[0].Set(x, y, z, x == 254 ? 0x01eeeeee : 0xeeeeee);
					}

					if (pow2f(static_cast<float>(x - 128)) + pow2f(static_cast<float>(y - 2 - 20)) + pow2f(static_cast<float>(z - 128)) <= pow2f(20))
					{
						voxels[0].Set(x, y, z, 0xaaffaa);
					}

					if (x >= 150 && x < 170 && y >= 2 && y < 22 && z >= 100 && z < 120)
					{
						voxels[0].Set(x, y, z, 0x5ffffff);
					}


				}
			}
		}
		//voxels[0].BuildBrickGrid();

		SetSphere(float3(200, 25, 50), 25.0f, 0x0400ff00);
		SetSphere(float3(25, 25, 25), 25.0f, 0xffffff);

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();

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
						voxels->Set(x, y, z, x == 510 ? 0x04eeeeee : 0xeeeeee);
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
							voxels->Set(x, y, z, color);
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

						voxels->Set(x, y, z, /*y == 1 || x == 1 || x == 510 || y == 510 || z == 510 ||*/ z == 1 ? 0x019999bb : 0xffffff);
						//Set(x, y, z, 0xeeeeee);
					}

					if (y > 30 && y < 50 && z > 50 && z < 70 && x > 20)
					{
						if (x > 55 && x < 75)
						{
							voxels->Set(x, y, z, 0x02ffffff);
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

		//voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)));
		//for (int z = 0; z < 512; z++)
		//{
		//	for (int x = 0; x < 512; x++)
		//	{
		//		for (int y = 0; y < 10; y++)   // platform thickness
		//		{
		//			uint color =
		//				(0x00 << 24) |   // diffuse
		//				(180 << 16) |
		//				(180 << 8) |
		//				180;
		//			voxels[0].Set(x, y, z, color);
		//		}
		//	}
		//}


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

		//tlas = new TLAS(voxels, voxelCount);
		//tlas->Build();
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
					uint8_t r = static_cast<uint8_t>(std::min(2 * y, 255));
					uint8_t g = static_cast<uint8_t>(std::min(1 * y, 255));
					uint8_t b = static_cast<uint8_t>(std::min(1 * y, 255));

					uint32_t c = (r << 16) | (g << 8) | b;
					voxels->Set(x, y, z, n > 0.09f ? c : 0);
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
						voxels->Set(x, y, z, stepColor);
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
					voxels->Set(x, y, z, wallColor);

		for (int y = 0; y < wallY + 40; y++)
			for (int z = baseZ; z < wallZ; z++)
				for (int x = baseX - 2; x < baseX; x++)
					voxels->Set(x, y, z, wallColor);

		for (int y = 0; y < wallY + 40; y++)
			for (int z = baseZ; z < wallZ; z++)
				for (int x = baseX + stepWidth; x < baseX + stepWidth + 2; x++)
					voxels->Set(x, y, z, wallColor);

		break;
	}
	case 9:
	{
		voxelCount = 2;
		voxels = new Voxel[voxelCount];
		// object 0: red cube on the left
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)));
		for (int x = 100; x < 200; x++)
			for (int y = 0; y < 100; y++)
				for (int z = 100; z < 200; z++)
					voxels[0].Set(x, y, z, 0xff0000);
		voxels[0].BuildBrickGrid();

		// object 1: blue cube on the right
		voxels[1].SetTransform(mat4::Translate(float3(2, 0, 0)));
		for (int x = 300; x < 400; x++)
			for (int y = 0; y < 100; y++)
				for (int z = 100; z < 200; z++)
					voxels[1].Set(x, y, z, 0x0000ff);
		voxels[1].BuildBrickGrid();

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 10:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)));
		//voxels[1].SetTransform(mat4::Translate(float3(256.0f / WORLDSIZE, 0, 0)));

		uint8_t roadX, roadY, roadZ;
		uint color;
		voxels[0].LoadFromFile("assets/binFiles/road.bin");

		for (int i = 0; i < voxelCount; i++)
		{
			voxels[i].BuildBrickGrid();
		}
		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	default:
		break;
	}


	//voxels->BuildBrickGrid();
	bvh = new BVH();
	bvh->BuildBVH(*this);
}

Scene::~Scene()
{
	delete bvh;
	delete[] voxels;
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

		//float sphereHit = 1e34f;
	//uint sphereHitMaterial = 0;
	//uint sphereHitAxis = 0;
	//uint sphereAxis = 0;

	if (tlas)
	{
		tlas->Intersect(ray);
	}

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

	if (voxels)
	{
		return voxels->IsOccluded(ray);
	}
	return false;
}