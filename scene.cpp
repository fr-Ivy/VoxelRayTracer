#include "template.h"

#include <set>

#include "bvh.h"
#include "voxel.h"
#include "TLAS.h"

Scene::Scene(int sceneIndex)
{
	this->sceneIndex = sceneIndex;
	switch (sceneIndex)
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
		voxels[0].Resize(512);
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
		voxelCount = 0;
		voxels = nullptr;
		tlas = nullptr;
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
				(0x01 << 24) |                 // diffuse material
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
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(256);
		// initialize the scene using Perlin noise, parallel over z
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 256; z++)
		{
			const float fz = (float)z / 256;
			for (int y = 0; y < 256; y++)
			{
				float fx = 0, fy = (float)y / 256;
				for (int x = 0; x < 256; x++, fx += 1.0f / 256)
				{
					const float n = noise3D(fx, fy, fz);
					uint8_t r = static_cast<uint8_t>(std::min(2 * y, 255));
					uint8_t g = static_cast<uint8_t>(std::min(1 * y, 255));
					uint8_t b = static_cast<uint8_t>(std::min(1 * y, 255));

					uint32_t c = (r << 16) | (g << 8) | b;
					voxels[0].Set(x, y, z, n > 0.09f ? c : 0);
				}
			}
		}
		voxels[0].SetTransform(mat4::Identity());
		voxels[0].BuildBrickGrid();

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 8:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(512);
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
						voxels[0].Set(x, y, z, stepColor);
		}

		float3 ballCenter;
		ballCenter.x = baseX + stepWidth / 2;
		ballCenter.y = stepCount * stepHeight + 10;
		ballCenter.z = baseZ + stepCount * stepDepth - 10;

		SetSphere(ballCenter, 12.0f, 0x843a99);

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
					voxels[0].Set(x, y, z, wallColor);

		for (int y = 0; y < wallY + 40; y++)
			for (int z = baseZ; z < wallZ; z++)
				for (int x = baseX - 2; x < baseX; x++)
					voxels[0].Set(x, y, z, wallColor);

		for (int y = 0; y < wallY + 40; y++)
			for (int z = baseZ; z < wallZ; z++)
				for (int x = baseX + stepWidth; x < baseX + stepWidth + 2; x++)
					voxels[0].Set(x, y, z, wallColor);
		voxels[0].BuildBrickGrid();
		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();

		break;
	}
	case 9:
	{
		voxelCount = 2;
		voxels = new Voxel[voxelCount];
		// object 0: red cube on the left
		voxels[0].Resize(512);
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)));
		for (int x = 100; x < 200; x++)
		{
			for (int y = 0; y < 100; y++)
			{
				for (int z = 100; z < 200; z++)
				{
					voxels[0].Set(x, y, z, 0xff0000);
				}
			}
		}
		voxels[0].BuildBrickGrid();

		// object 1: blue cube on the right
		voxels[1].Resize(512);
		voxels[1].SetTransform(mat4::Translate(float3(2, 0, 0)));
		for (int x = 300; x < 400; x++)
		{
			for (int y = 0; y < 100; y++)
			{
				for (int z = 100; z < 200; z++)
				{
					voxels[1].Set(x, y, z, 0x0000ff);
				}
			}
		}
		voxels[1].BuildBrickGrid();

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 10:
	{
		voxelCount = 2;
		voxels = new Voxel[voxelCount];
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)));
		voxels[1].SetTransform(mat4::Identity());
		voxels[0].Resize(64);
		voxels[1].Resize(64);
		//voxels[1].SetTransform(mat4::Translate(float3(256.0f / WORLDSIZE, 0, 0)));

		voxels[0].LoadFromFile("assets/binFiles/cat.bin");

		for (int i = 0; i < voxelCount; i++)
		{
			voxels[i].BuildBrickGrid();
		}

		for (int z = 0; z < 64; z++)
		{
			for (int y = 0; y < 8; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					voxels[1].Set(x, y, z, 0x00ff00);
				}
			}
		}

		float3 A = float3(0.2f, 0.5f, 0.2f);
		float3 B = float3(0.8f, 0.7f, 0.2f);
		float3 C = float3(0.8f, 0.3f, 0.8f);
		float3 D = float3(0.2f, 0.6f, 0.8f);

		voxels[1].AddSplineSegment(D, A, B, C, 2.0f);
		voxels[1].AddSplineSegment(A, B, C, D, 1.5f);
		voxels[1].AddSplineSegment(B, C, D, A, 1.5f);
		voxels[1].AddSplineSegment(C, D, A, B, 1.5f);

		voxels[1].SetTransform(mat4::Translate(A) * mat4::Scale(voxels[1].gridScale));

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 11:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(64);

		for (int z = 0; z < 8; z++)
		{
			for (int y = 0; y < 8; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					voxels[0].Set(x, y, z, 0x00ff00);
				}
			}
		}
		voxels[0].BuildBrickGrid();

		float3 A = float3(0.2f, 0.5f, 0.2f);
		float3 B = float3(0.8f, 0.7f, 0.2f);
		float3 C = float3(0.8f, 0.3f, 0.8f);
		float3 D = float3(0.2f, 0.6f, 0.8f);

		voxels[0].AddSplineSegment(D, A, B, C, 2.0f);
		voxels[0].AddSplineSegment(A, B, C, D, 1.5f);
		voxels[0].AddSplineSegment(B, C, D, A, 1.5f);
		voxels[0].AddSplineSegment(C, D, A, B, 1.5f);

		voxels[0].SetTransform(mat4::Translate(A));

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 12:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];

		float3 blockSize = float3(16, 16, 16);

		voxels[0].Resize(16);
		voxels[0].SetTransform(mat4::Scale(voxels[0].gridScale));


		for (int z = 0; z < blockSize.z; z++)
		{
			for (int y = 0; y < blockSize.y; y++)
			{
				for (int x = 0; x < blockSize.x; x++)
				{
					voxels[0].Set(x, y, z, 0x06570000);
				}
			}
		}

		voxels[0].BuildBrickGrid();

		//voxels[1].Resize(512);
		//	for (int z = 0; z < 512; z++)
		//	{
		//		for (int y = 0; y < 512; y++)
		//		{
		//			for (int x = 0; x < 512; x++)
		//			{
		//				if (x < 2 || x > 509 || z > 509 || y < 2 || y > 509 || z < 2)
		//				{
		//					voxels[1].Set(x, y, z, 0x06ff0000);
		//				}
		//			}
		//		}
		//	}
		//	voxels[1].BuildBrickGrid();




		float3 centerCube = float3(8 / 512.0f, 8 / 512.0f, 8 / 512.0f);
		float r = 20.0f / 512;

		float speeds[4] = { 5.0f, 3.5f, 7.0f, 4.5f };
		float radius[4] = { 25.0f, 35.0f, 43.0f, 50.0f };
		uint materials[4] = { 0x06ffffff, 0x01aaaaaa, 0x02ffffff, 0x04ff8844 };

		for (int i = 0; i < 4; i++)
		{
			float r = radius[i] / 512.0f;
			float3 center = centerCube;
			SetSphere(float3(80, 180, 80), 3, materials[i]);
			int idx = spheres.size() - 1;
			AddSplineSegment(idx, center + float3(0, 0, -r), center + float3(r, 0, 0), center + float3(0, 0, r), center + float3(-r, 0, 0), speeds[i]);
			AddSplineSegment(idx, center + float3(r, 0, 0), center + float3(0, 0, r), center + float3(-r, 0, 0), center + float3(0, 0, -r), speeds[i]);
			AddSplineSegment(idx, center + float3(0, 0, r), center + float3(-r, 0, 0), center + float3(0, 0, -r), center + float3(r, 0, 0), speeds[i]);
			AddSplineSegment(idx, center + float3(-r, 0, 0), center + float3(0, 0, -r), center + float3(r, 0, 0), center + float3(0, 0, r), speeds[i]);
		}

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 13:
	{
		voxelCount = 2;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(32);
		for (int z = 0; z < 32; z++)
		{
			for (int y = 0; y < 32; y++)
			{
				for (int x = 0; x < 32; x++)
				{
					if ((x < 1 || (x > 8 && x < 10)) && y < 14 ||
						(y < 1 || (y > 12 && y < 14)) && x < 10 ||
						(z < 1 || z > 30) && x < 10 && y < 14)
					{
						voxels[0].Set(x, y, z, 0xff00ff);
					}
				}
			}
		}

		int lightPositions[] = { 2, 15, 27 };
		for (int lights : lightPositions)
		{
			for (int z = lights; z < lights + 2; z++)
			{
				for (int x = 2; x < 8; x++)
				{
					voxels[0].Set(x, 12, z, 0x06ffffff);
				}
			}
		}
		voxels[0].BuildBrickGrid();

		voxels[1].Resize(16);
		for (int z = 0; z < 2; z++)
		{
			for (int y = 0; y < 2; y++)
			{
				for (int x = 0; x < 2; x++)
				{
					voxels[1].Set(x, y, z, 0x06ffffff);
				}
			}
		}
		voxels[1].SetTransform(mat4::Translate(float3(0.0075f, 0.01f, 0.005f)) * mat4::Scale(voxels[1].gridScale));
		voxels[1].BuildBrickGrid();

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 14:
	{
		voxelCount = 0;
		voxels = nullptr;
		tlas = nullptr;
		const int sphereCount = 1500;
		int placed = 0;

		const int curveSamples = 100;
		float heartX[curveSamples];
		float heartY[curveSamples];
		for (int i = 0; i < curveSamples; i++)
		{
			float t = static_cast<float>(i) / curveSamples * 2.0f * PI;
			heartX[i] = 16.0f * sinf(t) * sinf(t) * sinf(t);
			heartY[i] = 13.0f * cosf(t) - 5.0f * cosf(2 * t) - 2.0f * cosf(3 * t) - cosf(4 * t);
		}

		do
		{
			float x = (rand() % 1000) / 1000.0f * 36.0f - 18.0f;
			float y = (rand() % 1000) / 1000.0f * 34.0f - 18.0f;

			int crossings = 0;
			for (int i = 0; i < curveSamples; i++)
			{
				int j = (i + 1) % curveSamples;
				float ix = heartX[i];
				float iy = heartY[i];
				float jx = heartX[j];
				float jy = heartY[j];
				if (((iy > y) != (jy > y)) && (x < (jx - ix) * (y - iy) / (jy - iy) + ix))
				{
					crossings++;
				}
			}

			if (crossings % 2 == 1)
			{
				float3 heartPos(256.0f + x * 12.0f, 256.0f + y * 12.0f, 256.0f + rand() % 100);
				float3 randomPos(100.0f + rand() % 512, 100.0f + rand() % 512, 100.0f + rand() % 512);

				float radius = 2.0f + rand() % 3;

				float3 heartPosWorld = heartPos / 512.0f;
				float3 randomPosWorld = randomPos / 512.0f;

				SetSphere(randomPos, radius, 0x01ffffff);

				int idx = spheres.size() - 1;
				float3 offset = float3(0.001f, 0.001f, 0.001f);
				AddSplineSegment(idx, randomPosWorld - offset, randomPosWorld, heartPosWorld, heartPosWorld + offset, 24.0f);

				placed++;
			}

		} while (placed < sphereCount);

		break;
	}
	case 15:
	case 16:
	case 17:
	case 18:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(64);
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)) * mat4::Scale(voxels[0].gridScale));
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 64; z++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if ((x > 62 && y < 32) || y < 2)
					{
						voxels[0].Set(x, y, z, x == 63 ? 0x01eeeeee : 0xeeeeee);
					}

					if (x >= 28 && x < 36 && y >= 2 && y < 10 && z >= 28 && z < 36)
					{
						voxels[0].Set(x, y, z, 0x0200ff00);
					}
				}
			}
		}
		voxels[0].BuildBrickGrid();

		SetSphere(float3(32, 7, 50), 5.0f, 0xdebdff);

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 19:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(64);
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)) * mat4::Scale(voxels[0].gridScale));
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 64; z++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if ((x > 62 && y < 32) || y < 2)
					{
						voxels[0].Set(x, y, z, x == 63 ? 0x01eeeeee : 0xeeeeee);
					}

					if (x >= 28 && x < 36 && y >= 2 && y < 10 && z >= 28 && z < 36)
					{
						voxels[0].Set(x, y, z, 0x03ff0000);
					}
				}
			}
		}
		SetSphere(float3(32, 7, 50), 5.0f, 0xdebdff);
		voxels[0].BuildBrickGrid();


		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 20:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(64);
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)) * mat4::Scale(voxels[0].gridScale));
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 64; z++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (y < 2)
					{
						voxels[0].Set(x, y, z, 0xeeeeee);
					}

					if (x >= 28 && x < 36 && y >= 2 && y < 10 && z >= 28 && z < 36)
					{
						voxels[0].Set(x, y, z, 0xffffff);
					}
				}
			}
		}
		SetSphere(float3(32, 50, 100), 1.0f, 0x06ff0000);
		SetSphere(float3(-32, 50, -32), 1.0f, 0x0600ff00);
		SetSphere(float3(100, 50, 0), 1.0f, 0x060000ff);
		voxels[0].BuildBrickGrid();

		//SetSphere(float3(32, 5, 32), 5.0f, 0x04debdff);

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 21:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(64);
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)) * mat4::Scale(voxels[0].gridScale));
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 64; z++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (y < 2)
					{
						voxels[0].Set(x, y, z, 0xeeeeee);
					}

					if (x >= 28 && x < 36 && y >= 2 && y < 10 && z >= 28 && z < 36)
					{
						voxels[0].Set(x, y, z, 0xffffff);
					}
				}
			}
		}
		voxels[0].BuildBrickGrid();

		SetSphere(float3(32, 50, 32), 1, 0x06ffffff);

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 22:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(64);
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)) * mat4::Scale(voxels[0].gridScale));
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 64; z++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (y < 2)
					{
						voxels[0].Set(x, y, z, 0xeeeeee);
					}

					if (x >= 28 && x < 36 && y >= 2 && y < 10 && z >= 28 && z < 36)
					{
						voxels[0].Set(x, y, z, 0xffffff);
					}
				}
			}
		}
		SetSphere(float3(32, 50, 50), 1, 0x06ffffff);
		voxels[0].BuildBrickGrid();

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 23:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(64);
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)) * mat4::Scale(voxels[0].gridScale));
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 64; z++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (y < 2)
					{
						voxels[0].Set(x, y, z, 0xeeeeee);
					}

					if (x >= 28 && x < 36 && y >= 2 && y < 10 && z >= 28 && z < 36)
					{
						voxels[0].Set(x, y, z, 0xffffff);
					}
				}
			}
		}
		SetSphere(float3(32, 50, 50), 1, 0x06ff0000);
		voxels[0].BuildBrickGrid();

		//SetSphere(float3(32, 5, 32), 5.0f, 0x04debdff);

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 24:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(64);
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)) * mat4::Scale(voxels[0].gridScale));
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 64; z++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if ((x > 62 && y < 20) || y < 2)
					{
						voxels[0].Set(x, y, z, x == 63 ? 0x01eeeeee : 0xeeeeee);
					}

				}
			}
		}
		voxels[0].BuildBrickGrid();

		int sphereCount = 7;
		float radius = 3.0f;
		float spacing = 2.0f;
		float sphereRowWidth = static_cast<float>(sphereCount) * (radius * 2.0f) + static_cast<float>(sphereCount - 1) * spacing;
		float startX = 32.0f - sphereRowWidth / 2.0f + radius;

		for (int i = 0; i < sphereCount; i++)
		{
			float x = startX + static_cast<float>(i) * (radius * 2 + spacing);
			SetSphere(float3(x, 2 + radius, 32), radius, 0x940101);
		}

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	{
		voxelCount = 1;
		voxels = new Voxel[voxelCount];
		voxels[0].Resize(64);
		voxels[0].SetTransform(mat4::Translate(float3(0, 0, 0)) * mat4::Scale(voxels[0].gridScale));
#pragma omp parallel for schedule(dynamic)
		for (int z = 0; z < 64; z++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (y < 2)
					{
						voxels[0].Set(x, y, z, 0xeeeeee);
					}

					if (x >= 15 && x < 20 && y >= 2 && y < 7 && z >= 30 && z < 35)
					{
						//voxels[0].Set(x, y, z, 0x0500ff00);
					}
				}
			}
		}
		voxels[0].BuildBrickGrid();

		SetSphere(float3(32, 7, 32), 5.0f, 0x04ff4fed);

		for (int i = 0; i < 100; i++)
		{
			float3 randomPos(-100.0f + rand() % 256, -50.0f + rand() % 256, -100.0f + rand() % 256);

			float radius = 6.0f + rand() % 3;

			//float3 randomPosWorld = randomPos / 512.0f;

			uint color =
				(0x06 << 24) |
				((rand() % 256) << 16) |
				((rand() % 256) << 8) |
				(rand() % 256);

			SetSphere(randomPos, radius, color);
		}

		tlas = new TLAS(voxels, voxelCount);
		tlas->Build();
		break;
	}

	default:
		break;
	}

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

void Scene::AddSplineSegment(int sphereIndex, float3 p0, float3 p1, float3 p2, float3 p3, float duration)
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
	spheres[sphereIndex].splineSegments.push_back(segment);
}

float3 Scene::EvaluateSpline(Sphere& sphere, float t)
{
	for (auto& segment : sphere.splineSegments)
	{
		if (t <= segment.duration)
		{
			float progress = t / segment.duration;
			return segment.a * progress * progress * progress + segment.b * progress * progress + segment.c * progress + segment.d;
		}

		t -= segment.duration;
	}
	return sphere.splineSegments.back().d;
}

void Scene::UpdateSphereSpline(float deltaTime)
{
	for (auto& sphere : spheres)
	{
		if (sphere.splineSegments.empty())
		{
			continue;
		}

		sphere.splineTime += deltaTime;

		float totalDuration = 0.0f;
		for (auto& segment : sphere.splineSegments)
		{
			totalDuration += segment.duration;
		}
		if (sphere.splineTime > totalDuration)
		{
			sphere.splineTime -= totalDuration;
		}

		sphere.center = EvaluateSpline(sphere, sphere.splineTime);
	}
}


void Scene::FindNearest(Ray& ray, bool skipBVH, bool skipTLAS) const
{
	// nudge origin
	ray.O += EPSILON * ray.D;

	if (!skipBVH && bvh)
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

	if (tlas && !skipTLAS)
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

	//ray.hitSphere = false;

	//setup shadows for the spheres

	if (bvh && !bvh->bvhNodes.empty())
	{
		bvh->IntersectBVH(ray, *this);
		if (ray.hitSphere && (ray.voxel >> 24) != 6)
		{
			return true;
		}
	}


	if (tlas)
	{
		return tlas->IsOccluded(ray);
	}
	return false;
}