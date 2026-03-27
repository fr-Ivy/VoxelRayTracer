#include "template.h"

#include "bvh.h"
#include "Dielectric.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "QuadLight.h"
#include "SpotLight.h"
#include "Reflective.h"
#include "Refractive.h"
#include "Dielectric.h"
#include "Hybrid.h"
#include "stb_image.h"
#include "RollBall.h"
#include "Voxel.h"
#include "TLAS.h"

// -----------------------------------------------------------
// Calculate light transport via a ray
// -----------------------------------------------------------

float3 Renderer::Trace(Ray& ray, int depth, int, int /* we'll use these later */)
{
	if (depth == maxDepth)
	{
		return 0;
	}

	scene.FindNearest(ray);
	if (ray.voxel == 0)
	{
		return SampleSky(ray.D);
		//return float3(0, 1, 1); // or a fancy sky color
	}

	float3 N = ray.GetNormal();
	float3 I = ray.IntersectionPoint();
	float3 albedo = ray.GetAlbedo();

	float3 lighting = Shade(N, I);

	//reflective material
	if ((ray.voxel >> 24) == 1)
	{
		float3 reflected = reflectiveMat->calc(*this, ray, N, I, depth);
		return reflected;
	}

	//dielectric material
	else if ((ray.voxel >> 24) == 2)
	{
		float3 transmitted = dielectricMat->calc(*this, ray, N, I, depth);
		return albedo * transmitted /** lighting*/;
	}

	//refractive material
	else if ((ray.voxel >> 24) == 3)
	{
		float3 refracted = refractiveMat->calc(*this, ray, N, I, depth);
		return albedo * refracted * lighting;
	}

	//hybrid material
	else if ((ray.voxel >> 24) == 4)
	{
		std::shared_ptr<Hybrid> hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);
		float3 reflected = hybrid->calc(*this, ray, N, I, depth);
		return (1.0f - hybrid->specular) * albedo * lighting + hybrid->specular * reflected;
	}

	//textured material
	else if ((ray.voxel >> 24) == 5)
	{
		return Triplanar(I, N) * lighting;
	}

	else if ((ray.voxel >> 24) == 6)
	{
		return albedo;
	}

	//default
	else
	{
		return albedo * lighting;
		//return albedo * max(0.3f, dot(N, I));
	}
}

float3 Renderer::Shade(const float3& N, const float3& I)
{
	float3 color = float3(0.15f);

	for (AreaLight* light : areaLights)
	{
		// for area lights
		float3 direction, emission;
		float distance, pdf;

		if (light->sample(I, float2(RandomFloat(), RandomFloat()), direction, emission, distance, pdf))
		{
			if (dot(N, direction) > 0.0f)
			{
				Ray shadow(I, direction, distance);
				if (!scene.IsOccluded(shadow))
				{
					float surface = max(0.0f, dot(N, direction));
					float3 lightNormal = light->getNormal();
					float lightDistance = max(0.0f, -dot(direction, lightNormal));

					float geometry = (surface * lightDistance) / (distance * distance);

					color += emission * geometry / pdf;
				}
			}
		}
	}

	// for directional light
	float3 lightDirection = directionalLight->SampleDirection(I);

	if (dot(N, lightDirection) > 0.0f)
	{
		if (!directionalLight->IsOccluded(I, scene))
		{
			float3 lightRadiance = directionalLight->Radiance(I);
			float cosa = max(0.0f, dot(N, lightDirection));
			color += lightRadiance * cosa;
		}
	}

	// for point lights
	for (PointLight& light : pointLights)
	{
		// for other lights
		lightDirection = light.SampleDirection(I);

		if (dot(N, lightDirection) > 0.0f)
		{
			if (!light.IsOccluded(I, scene))
			{
				float3 lightRadiance = light.Radiance(I);
				float cosa = max(0.0f, dot(N, lightDirection));
				color += lightRadiance * cosa;
			}
		}
	}

	// for spotlights
	for (PointLight& light : pointLights)
	{
		//for other lights
		lightDirection = light.SampleDirection(I);

		if (dot(N, lightDirection) > 0.0f)
		{
			if (!light.IsOccluded(I, scene))
			{
				float3 lightRadiance = light.Radiance(I);
				float cosa = max(0.0f, dot(N, lightDirection));
				color += lightRadiance * cosa;
			}
		}
	}

	return color;
}

// -----------------------------------------------------------
// Application initialization - Executed once, at app start
// -----------------------------------------------------------
void Renderer::Init()
{
	accumulator = new float3[SCRWIDTH * SCRHEIGHT];
	memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));

	reflectiveMat = std::make_shared<Reflective>();
	refractiveMat = std::make_shared<Refractive>();
	dielectricMat = std::make_shared<Dielectric>();
	hybridMat = std::make_shared<Hybrid>(1.0f, 0.1f);

	directionalLight = new DirectionalLight(normalize(float3(0.2f, -0.5, 1)), float3(1, 1, 1));

	//pointLights.push_back(PointLight(float3(0.033f, 0.051f, 0.124f), float3(1, 0, 0)));
	//lights.push_back(new PointLight(float3(0.5f, 0.7f, 1.0f), float3(0, 1, 0)));
	//lights.push_back(new PointLight(float3(0.8f, 0.7f, 0.5f), float3(0, 0, 1)));
	//pointLights.push_back(PointLight(float3(0.5f, 0.5f, 0.5f), float3(0, 1, 1)));

	for (int i = -15; i < 15; i++)
	{
		//lights.push_back(new PointLight(float3(static_cast<float>(i) / 10, 0.7f, 0.6f), float3(1, 0, 0)));
	}

	texture = new Surface("assets/earthmap.jpg");
	skydome = new Surface("assets/skydome2_4K.hdr");

	//lights.push_back(new SpotLight(float3(0.5f, 0.1f, 1.0f), float3(0, -1, 0), float3(1, 1, 1), 10, 13));

	areaLights.push_back(new QuadLight(float3(0.033f - 0.025f, 0.093f, 0.124f - 0.025f), float3(0.05f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.05f), float3(1, 1, 1)));
}

// -----------------------------------------------------------
// Main application tick function - Executed every frame
// -----------------------------------------------------------

static int spp = 1;

void Renderer::Tick(float deltaTime)
{
	audio.Play();
	if (playCameraAnimation)
	{
		float cameraDuration = 8.0f;
		cameraTime += (deltaTime / 1000.0f);
		float time = cameraTime / cameraDuration;

		if (time > 1.0f)
		{
			cameraTime = 0.0f;
		}

		const int pointsCount = 4;

		float3 cameraPoints[4] =
		{
			float3(0.5f + 1.2f, 0.5f, 0.5f),
			float3(0.5f, 0.5f, 0.5f + 1.2f),
			float3(0.5f - 1.2f, 0.5f, 0.5f),
			float3(0.5f, 0.5f, 0.5f - 1.2f)
		};

		int   segment = static_cast<int>(time * pointsCount);
		float t = time * pointsCount - segment;

		float3 p0 = cameraPoints[(segment - 1 + pointsCount) % pointsCount];
		float3 p1 = cameraPoints[(segment + pointsCount) % pointsCount];
		float3 p2 = cameraPoints[(segment + 1 + pointsCount) % pointsCount];
		float3 p3 = cameraPoints[(segment + 2 + pointsCount) % pointsCount];

		camera.CatmullRomSplines(p0, p1, p2, p3, t);
		changedSetting = true;
	}

	if (playObjectAnimation)
	{
		for (int i = 0; i < 16; i++)
		{
			scene.voxels[i].UpdateSpline(deltaTime / 1000.0f);

		}
		scene.tlas->Build();
		changedSetting = true;
	}


	if (physics)
	{
		for (int i = 0; i < scene.spheres.size(); i++)
		{
			rollBall.Move(deltaTime, scene.spheres[i], scene);
			changedSetting = true;
		}
		scene.bvh->BuildBVH(scene);
	}


	//	Timer t;
	//	// pixel loop: lines are executed as OpenMP parallel tasks (disabled in DEBUG)
	//#pragma omp parallel for schedule(dynamic)
	//	for (int y = 0; y < SCRHEIGHT; y++)
	//	{
	//		// trace a primary ray for each pixel on the line
	//		for (int x = 0; x < SCRWIDTH; x++)
	//		{
	//			Ray r = camera.GetPrimaryRay((float)x, (float)y);
	//			float3 pixel = Trace(r);
	//			screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(pixel);
	//		}
	//	}
	//	// performance report - running average - ms, MRays/s
	//	static float avg = 10, alpha = 1;
	//	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	//	if (alpha > 0.05f) alpha *= 0.5f;
	//	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	//	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
	//	// handle user input
	//	camera.HandleInput(deltaTime);

		// high-resolution timer, see template.h
	Timer t;
	const float scale = 1.0f / spp++;
	// pixel loop: lines are executed as OpenMP parallel tasks (disabled in DEBUG)
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++)
	{
		// trace a primary ray for each pixel on the line
		for (int x = 0; x < SCRWIDTH; x++)
		{
			Ray r = camera.GetPrimaryRay(x + RandomFloat(), y + RandomFloat());
			accumulator[x + y * SCRWIDTH] += Trace(r);
			float3 average = accumulator[x + y * SCRWIDTH] * scale;
			screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(average);
		}
	}
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	fps = 1000.0f / avg;
	float rps = (SCRWIDTH * SCRHEIGHT) / avg;
	if (spp % 20 == 0)
	{
		printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
	}
	// handle user input
	if (camera.HandleInput(deltaTime) || changedSetting)
	{
		spp = 1;
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
	}
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI()
{
	changedSetting = false;
	// ray query on mouse
	Ray r = camera.GetPrimaryRay((float)mousePos.x, (float)mousePos.y);
	float3 cameraPos = camera.camPos;
	scene.FindNearest(r);
	ImGui::StyleColorsLight();

	if (ImGui::BeginTabBar("Debug"))
	{
		if (ImGui::BeginTabItem("Info"))
		{
			ImGui::Text("voxel: %i", r.voxel);
			ImGui::Text("FPS: %.1f", fps);
			ImGui::Text("position: %.1f, %.1f, %.1f", cameraPos.x, cameraPos.y, cameraPos.z);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	if (ImGui::BeginTabBar("General"))
	{
		if (ImGui::BeginTabItem("General"))
		{
			changedSetting |= ImGui::Checkbox("shadows", &scene.shadows);
			changedSetting |= ImGui::Checkbox("physics", &physics);
			changedSetting |= ImGui::Checkbox("play camera animation", &playCameraAnimation);
			changedSetting |= ImGui::Checkbox("play object animation", &playObjectAnimation);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	if (ImGui::BeginTabBar("Scene"))
	{
		if (ImGui::BeginTabItem("Scene"))
		{
			if (ImGui::Button("Hallway"))
			{
				scene.~Scene();
				new (&scene) Scene(13);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("moving platforms"))
			{
				scene.~Scene();
				new (&scene) Scene(12);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("1000 spheres"))
			{
				scene.~Scene();
				new (&scene) Scene(6);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	if (ImGui::BeginTabBar("Camera"))
	{
		if (ImGui::BeginTabItem("general"))
		{
			pinhole = camera.cameraMode == CAMERAMODE::PINHOLE;
			if (ImGui::Checkbox("Pinhole", &pinhole))
			{
				camera.cameraMode = CAMERAMODE::PINHOLE;
				changedSetting = true;
			}

			ImGui::SameLine();

			panini = camera.cameraMode == CAMERAMODE::PANINI;
			if (ImGui::Checkbox("Panini", &panini))
			{
				camera.cameraMode = CAMERAMODE::PANINI;
				changedSetting = true;
			}

			ImGui::SameLine();

			fishEye = camera.cameraMode == CAMERAMODE::FISHEYE;
			if (ImGui::Checkbox("FishEye", &fishEye))
			{
				camera.cameraMode = CAMERAMODE::FISHEYE;
				changedSetting = true;
			}


			if (ImGui::Checkbox("aperture", &camera.aperture))
			{
				if (!camera.aperture)
				{
					camera.apertureRadius = 0.0f;
					changedSetting = true;
				}
				else
				{
					camera.focusDistance = 0.2f;
					camera.apertureRadius = 0.003f;
					changedSetting = true;
				}
			}

			if (camera.aperture)
			{
				changedSetting |= ImGui::InputFloat("focusDistance", &camera.focusDistance, 0.07f, 3.0f);
				changedSetting |= ImGui::InputFloat("aperture radius", &camera.apertureRadius, 0.0f, 0.1f);
			}
			else
			{
				camera.apertureRadius = 0.0f;
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Panini"))
		{
			changedSetting |= ImGui::SliderFloat("Strength", &camera.paniniStrength, 0.0f, 1.0f);

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	if (ImGui::BeginTabBar("Lighting"))
	{
		if (ImGui::BeginTabItem("Directional"))
		{
			changedSetting |= ImGui::SliderFloat("Position x", &directionalLight->direction.x, -1.0f, 1.0f);
			//ImGui::SameLine();
			changedSetting |= ImGui::SliderFloat("Position y", &directionalLight->direction.y, -1.0f, 1.0f);
			//ImGui::SameLine();
			changedSetting |= ImGui::SliderFloat("Position z", &directionalLight->direction.z, -1.0f, 1.0f);

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	if (ImGui::BeginTabBar("Material"))
	{
		if (ImGui::BeginTabItem("general"))
		{
			changedSetting |= ImGui::SliderInt("depth", &maxDepth, 1, 20);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Hybrid"))
		{
			std::shared_ptr<Hybrid> hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);

			if (hybrid)
			{
				changedSetting |= ImGui::SliderFloat("specular", &hybrid->specular, 0, 1);
				changedSetting |= ImGui::SliderFloat("roughness", &hybrid->roughness, 0, 1);
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("dielectric"))
		{
			std::shared_ptr<Dielectric> dielectric = std::dynamic_pointer_cast<Dielectric>(dielectricMat);

			if (dielectric)
			{
				if (ImGui::Button("glass"))
				{
					dielectric->index2 = 1.46f;

					changedSetting = true;
				}

				ImGui::SameLine();

				if (ImGui::Button("water"))
				{
					dielectric->index2 = 1.33f;

					changedSetting = true;
				}

				ImGui::SameLine();

				if (ImGui::Button("diamond"))
				{
					dielectric->index2 = 2.40f;

					changedSetting = true;
				}
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

float3 Renderer::SampleSky(float3& distance)
{
	float u = atan2f(distance.z, distance.x) * INV2PI + 0.5f;
	float v = acosf(distance.y) * INVPI;

	//v = 1.0f - v;

	int iu = static_cast<int>(u * static_cast<float>(skydome->width));
	int iv = static_cast<int>(v * static_cast<float>(skydome->height));

	iu = max(0, min(iu, skydome->width - 1));
	iv = max(0, min(iv, skydome->height - 1));

	uint p = skydome->pixels[iu + iv * skydome->width];

	float r = ((p >> 16) & 255) / 255.0f;
	float g = ((p >> 8) & 255) / 255.0f;
	float b = (p & 255) / 255.0f;
	return float3(r, g, b);
}

float3 Renderer::SampleTexture(float u, float v)
{
	if (!texture || !texture->pixels)
	{
		return float3(1, 0, 0);
	}

	u = u - floor(u);
	v = v - floor(v);

	int x = static_cast<int>(u * texture->width);
	int y = static_cast<int>(v * texture->height);

	x = clamp(x, 0, texture->width - 1);
	y = clamp(y, 0, texture->height - 1);

	uint pixel = texture->pixels[x + y * texture->width];

	float r = ((pixel >> 16) & 255) / 255.0f;
	float g = ((pixel >> 8) & 255) / 255.0f;
	float b = (pixel & 255) / 255.0f;

	return float3(r, g, b);
}


float3 Renderer::Triplanar(float3 I, float3 N)
{
	float scale = 25.0f;
	float3 local = I * scale;

	float3 n = float3(fabs(N.x), fabs(N.y), fabs(N.z));
	float sum = n.x + n.y + n.z;
	float bx = n.x / sum;
	float by = n.y / sum;
	float bz = n.z / sum;


	//X projection for YZ plane
	float3 cX = SampleTexture(local.y, local.z);

	//Y projection for XZ plane
	float3 cY = SampleTexture(local.x, local.z);

	//Z projection for XY plane
	float3 cZ = SampleTexture(local.x, local.y);

	return cX * bx + cY * by + cZ * bz;
}
