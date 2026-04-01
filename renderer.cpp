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

float3 Renderer::Trace(Ray& ray, int const depth, int, int /* we'll use these later */)
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

	float3 const N = ray.GetNormal();
	float3 const I = ray.IntersectionPoint();
	float3 const albedo = ray.GetAlbedo();

	if ((ray.voxel >> 24) == 6)
	{
		return albedo;
	}

	float3 const lighting = Shade(N, I);

	//reflective material
	if ((ray.voxel >> 24) == 1)
	{
		float3 const reflected = reflectiveMat->Calc(*this, ray, N, I, depth);
		return reflected;
	}

	//dielectric material
	else if ((ray.voxel >> 24) == 2)
	{
		float3 const transmitted = dielectricMat->Calc(*this, ray, N, I, depth);
		return albedo * transmitted /** lighting*/;
	}

	//refractive material
	else if ((ray.voxel >> 24) == 3)
	{
		float3 const refracted = refractiveMat->Calc(*this, ray, N, I, depth);
		return albedo * refracted /** lighting*/;
	}

	//hybrid material
	else if ((ray.voxel >> 24) == 4)
	{
		std::shared_ptr<Hybrid> const hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);
		float3 const reflected = hybrid->Calc(*this, ray, N, I, depth);
		return (1.0f - hybrid->specular) * albedo * lighting + hybrid->specular * reflected;
	}

	//textured material
	else if ((ray.voxel >> 24) == 5)
	{
		return Triplanar(I, N) * lighting;
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
	float3 color = float3(0.0f);
	if (!areaLights.empty())
	{
		int lightIndex = min(static_cast<int>(RandomFloat() * static_cast<float>(areaLights.size())), static_cast<int>(areaLights.size() - 1));
		AreaLight* light = areaLights[lightIndex];

		// for area lights
		float3 direction, emission;
		float distance, pdf;

		if (light->Sample(I, float2(RandomFloat(), RandomFloat()), direction, emission, distance, pdf))
		{
			if (dot(N, direction) > 0.0f)
			{
				Ray shadow(I, direction, distance);
				if (!scene.IsOccluded(shadow))
				{
					float surface = max(0.0f, dot(N, direction));
					float3 lightNormal = light->GetNormal();
					float lightDistance = max(0.0f, -dot(direction, lightNormal));

					float geometry = (surface * lightDistance) / (distance * distance);

					color += emission * geometry / pdf * static_cast<float>(areaLights.size());
				}
			}
		}
	}

	float3 lightDirection = float3(0);

	// for directional light
	if (directionalLight->color.x > 0 || directionalLight->color.y > 0 || directionalLight->color.z > 0)
	{
		lightDirection = directionalLight->SampleDirection();

		if (dot(N, lightDirection) > 0.0f)
		{
			if (!directionalLight->IsOccluded(I, scene))
			{
				float3 lightRadiance = directionalLight->Radiance();
				float cosa = max(0.0f, dot(N, lightDirection));
				color += lightRadiance * cosa;
			}
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
	for (SpotLight& light : spotLights)
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

void Renderer::SetupScenes(int sceneIndex)
{
	areaLights.clear();
	pointLights.clear();
	spotLights.clear();
	camera.splineSegments.clear();
	camera.splineTime = 0.0f;

	directionalLight->direction = float3(0);
	directionalLight->color = float3(0);

	switch (sceneIndex)
	{
	case 6:
	{
		maxDepth = 3;
		scene.shadows = true;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		break;
	}
	case 7:
	{
		maxDepth = 1;
		scene.shadows = true;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		pointLights.push_back(PointLight(float3(0.5f, 0.7f, 1.0f), float3(0, 1, 0)));
		break;
	}
	case 8:
	{
		camera.aperture = false;
		camera.apertureRadius = 0.0f;
		maxDepth = 1;
		scene.shadows = true;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		float3 A = float3(226 / 512.0f, 50 / 512.0f, -0 / 512.0f);
		float3 B = float3(226 / 512.0f, 50 / 512.0f, -0 / 512.0f);

		camera.AddSplineSegment(B, A, B, A, 2.5f);
		camera.AddSplineSegment(A, B, A, B, 2.5f);
		break;
	}
	case 12:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Dielectric> dielectric = std::dynamic_pointer_cast<Dielectric>(dielectricMat);
		dielectric->index2 = 1.46f;

		float3 A = float3(30 / 512.0f, 40 / 512.0f, -60 / 512.0f);
		float3 B = float3(-80 / 512.0f, 60 / 512.0f, -100 / 512.0f);

		camera.AddSplineSegment(A, A, B, B, 21.5f);
		camera.splineTime = -1.5f;
		break;
	}
	case 13:
	{
		camera.aperture = false;
		camera.apertureRadius = 0.0f;
		camera.cameraMode = CAMERAMODE::PINHOLE;
		scene.shadows = false;
		maxDepth = 1;
		float3 A = float3(0.0075f, 0.012f, 0.13f / 4);
		float3 B = float3(0.0075f, 0.012f, 0.23f / 4);

		camera.AddSplineSegment(B, A, B, A, 26.5f);

		int lightPositions[] = { 2, 15, 27 };
		for (int lights : lightPositions)
		{
			areaLights.push_back(new QuadLight(
				float3(1.0f / 512, 12.0f / 512, static_cast<float>(lights) / 512.0f),
				float3(7.0f / 512, 0.0f, 0.0f),
				float3(0.0f, 0.0f, 2.0f / 512),
				float3(1, 1, 1)));
		}
		break;
	}
	case 14:
	{
		scene.shadows = true;
		maxDepth = 4;
		float3 A = float3(1.0f, 1.0f, 1.7f);
		float3 B = float3(0.8f, 0.8f, 1.5f);

		camera.AddSplineSegment(B, A, B, A, 24.0f);
		break;
	}
	case 15:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Dielectric> dielectric = std::dynamic_pointer_cast<Dielectric>(dielectricMat);
		dielectric->index2 = 1.46f;

		float3 A = float3(50 / 512.0f, 25 / 512.0f, -80 / 512.0f);
		float3 B = float3(-120 / 512.0f, 25 / 512.0f, -10 / 512.0f);

		camera.AddSplineSegment(B, A, B, A, 2.75f);
		camera.AddSplineSegment(A, B, A, B, 2.75f);

		break;
	}
	case 16:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Dielectric> dielectric = std::dynamic_pointer_cast<Dielectric>(dielectricMat);
		dielectric->index2 = 1.33f;

		float3 A = float3(50 / 512.0f, 25 / 512.0f, -80 / 512.0f);
		float3 B = float3(-120 / 512.0f, 25 / 512.0f, -10 / 512.0f);

		camera.AddSplineSegment(B, A, B, A, 2.75f);
		camera.AddSplineSegment(A, B, A, B, 2.75f);
		break;
	}
	case 17:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Dielectric> dielectric = std::dynamic_pointer_cast<Dielectric>(dielectricMat);
		dielectric->index2 = 2.40f;

		float3 A = float3(50 / 512.0f, 25 / 512.0f, -80 / 512.0f);
		float3 B = float3(-120 / 512.0f, 25 / 512.0f, -10 / 512.0f);

		camera.AddSplineSegment(B, A, B, A, 2.5f);
		camera.AddSplineSegment(A, B, A, B, 2.5f);
		break;
	}
	case 18:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Dielectric> dielectric = std::dynamic_pointer_cast<Dielectric>(dielectricMat);
		dielectric->index2 = 1.00f;

		float3 A = float3(50 / 512.0f, 25 / 512.0f, -80 / 512.0f);
		float3 B = float3(-120 / 512.0f, 25 / 512.0f, -10 / 512.0f);

		camera.AddSplineSegment(B, A, B, A, 2.5f);
		camera.AddSplineSegment(A, B, A, B, 2.5f);
		break;
	}
	case 19:
	{
		scene.shadows = true;
		maxDepth = 8;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);

		float3 A = float3(50 / 512.0f, 25 / 512.0f, -80 / 512.0f);
		float3 B = float3(-120 / 512.0f, 25 / 512.0f, -10 / 512.0f);

		camera.AddSplineSegment(B, A, B, A, 2.5f);
		camera.AddSplineSegment(A, B, A, B, 2.5f);

		break;
	}
	case 20:
	{
		scene.shadows = true;
		maxDepth = 4;
		pointLights.push_back(PointLight(float3(32.0f / 512, 50.0f / 512, 100.0f / 512), float3(1, 0, 0)));
		pointLights.push_back(PointLight(float3(-32.0f / 512, 50.0f / 512, -32.0f / 512), float3(0, 1, 0)));
		pointLights.push_back(PointLight(float3(100.0f / 512, 50.0f / 512, 0), float3(0, 0, 1)));
		skydome = skydome2;

		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.5f);
		camera.AddSplineSegment(A, B, C, D, 1.5f);
		camera.AddSplineSegment(B, C, D, A, 1.5f);
		camera.AddSplineSegment(C, D, A, B, 1.5f);
		break;
	}
	case 21:
	{
		scene.shadows = true;
		maxDepth = 4;
		spotLights.push_back(SpotLight(float3(32.0f / 512, 50.0f / 512, 32.0f / 512), float3(0, -1, 0), float3(1, 1, 1), 10, 20));
		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.25f);
		camera.AddSplineSegment(A, B, C, D, 1.25f);
		camera.AddSplineSegment(B, C, D, A, 1.25f);
		camera.AddSplineSegment(C, D, A, B, 1.25f);
		break;
	}
	case 22:
	{
		scene.shadows = true;
		maxDepth = 4;
		spotLights.push_back(SpotLight(float3(32.0f / 512, 50.0f / 512, 50.0f / 512), float3(0, -1, -0.5), float3(1, 1, 1), 10, 20));
		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.25f);
		camera.AddSplineSegment(A, B, C, D, 1.25f);
		camera.AddSplineSegment(B, C, D, A, 1.25f);
		camera.AddSplineSegment(C, D, A, B, 1.25f);
		break;
	}
	case 23:
	{
		scene.shadows = true;
		maxDepth = 4;
		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.25f);
		camera.AddSplineSegment(A, B, C, D, 1.25f);
		camera.AddSplineSegment(B, C, D, A, 1.25f);
		camera.AddSplineSegment(C, D, A, B, 1.25f);
		spotLights.push_back(SpotLight(float3(32.0f / 512, 50.0f / 512, 50.0f / 512), float3(0, -1, -0.5), float3(1, 0, 0), 10, 20));
		break;
	}
	case 24:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(0.2f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		camera.aperture = true;
		camera.focusDistance = 0.2f;
		camera.apertureRadius = 0.003f;
		changedSetting = true;
		float3 A = float3(-0.15f, 0.05f, -0.05f);
		camera.AddSplineSegment(A, A, A, A, 10.0f);
		break;
	}
	case 25:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(-1.0f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		camera.aperture = false;
		camera.apertureRadius = 0.0f;
		std::shared_ptr<Hybrid> hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);
		hybrid->roughness = 0.0f;
		hybrid->specular = 1.0f;
		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.25f);
		camera.AddSplineSegment(A, B, C, D, 1.25f);
		camera.AddSplineSegment(B, C, D, A, 1.25f);
		camera.AddSplineSegment(C, D, A, B, 1.25f);
		changedSetting = true;
		break;
	}
	case 26:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(-1.0f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Hybrid> hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);
		hybrid->roughness = 0.0f;
		hybrid->specular = 0.5f;
		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.375f);
		camera.AddSplineSegment(A, B, C, D, 1.375f);
		camera.AddSplineSegment(B, C, D, A, 1.375f);
		camera.AddSplineSegment(C, D, A, B, 1.375f);
		changedSetting = true;
		break;
	}
	case 27:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(-1.0f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Hybrid> hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);
		hybrid->roughness = 0.0f;
		hybrid->specular = 0.25f;
		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.375f);
		camera.AddSplineSegment(A, B, C, D, 1.375f);
		camera.AddSplineSegment(B, C, D, A, 1.375f);
		camera.AddSplineSegment(C, D, A, B, 1.375f);
		changedSetting = true;
		break;
	}
	case 28:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(-1.0f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Hybrid> hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);
		hybrid->roughness = 0.1f;
		hybrid->specular = 1.0f;
		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.25f);
		camera.AddSplineSegment(A, B, C, D, 1.25f);
		camera.AddSplineSegment(B, C, D, A, 1.25f);
		camera.AddSplineSegment(C, D, A, B, 1.25f);
		changedSetting = true;
		break;
	}
	case 29:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(-1.0f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Hybrid> hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);
		hybrid->roughness = 0.25f;
		hybrid->specular = 1.0f;
		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.25f);
		camera.AddSplineSegment(A, B, C, D, 1.25f);
		camera.AddSplineSegment(B, C, D, A, 1.25f);
		camera.AddSplineSegment(C, D, A, B, 1.25f);
		changedSetting = true;
		break;
	}
	case 30:
	{
		scene.shadows = true;
		maxDepth = 4;
		directionalLight->direction = normalize(float3(-1.0f, -0.5f, 1.0f));
		directionalLight->color = float3(1);
		std::shared_ptr<Hybrid> hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);
		hybrid->roughness = 0.1f;
		hybrid->specular = 0.5f;
		float3 A = float3(-81 / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 B = float3((81 + 64) / 512.0f, 70 / 512.0f, -81 / 512.0f);
		float3 C = float3((81 + 64) / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);
		float3 D = float3(-81 / 512.0f, 70 / 512.0f, (81 + 64) / 512.0f);

		camera.AddSplineSegment(D, A, B, C, 1.25f);
		camera.AddSplineSegment(A, B, C, D, 1.25f);
		camera.AddSplineSegment(B, C, D, A, 1.25f);
		camera.AddSplineSegment(C, D, A, B, 1.25f);
		changedSetting = true;
		break;
	}
	default:
		break;
	}
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

	texture = new Surface("assets/texture.jpg");
	skydome = new Surface("assets/skydome2_4K.hdr");
	skydome2 = new Surface("assets/rogland_clear_night_2k.hdr");

	//lights.push_back(new SpotLight(float3(0.5f, 0.1f, 1.0f), float3(0, -1, 0), float3(1, 1, 1), 10, 13));

	SetupScenes(13);
}

// -----------------------------------------------------------
// Main application tick function - Executed every frame
// -----------------------------------------------------------

static int spp = 1;

void Renderer::Tick(float deltaTime)
{
	if (playDemo)
	{
		audio.Play();
		demoTime += deltaTime * 0.001f;

		static constexpr float sceneChangesForBlackFade[] = { 26.5f, 48.0f, 69.0f, 93.0f, 120.0f, 114.0f, 125.0f, 130.5f, 136.0f, 141.0f, 146.0f, 151.0f, 157.0f };

		constexpr float fadeDuration = 0.75f;
		fadeFactor = 1.0f;
		for (float sceneChange : sceneChangesForBlackFade)
		{
			float distance = fabsf(demoTime - sceneChange);
			if (distance < fadeDuration)
			{
				fadeFactor = min(fadeFactor, distance / fadeDuration);
			}
		}

		if (demoTime < 26.5f)
		{
			if (scene.sceneIndex != 13)
			{
				scene.~Scene();
				new (&scene) Scene(13);
				SetupScenes(13);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 48.0f)
		{
			if (scene.sceneIndex != 12)
			{
				scene.~Scene();
				new (&scene) Scene(12);
				SetupScenes(12);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 53.5f)
		{
			if (scene.sceneIndex != 15)
			{
				scene.~Scene();
				new (&scene) Scene(15);
				SetupScenes(15);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 59.0f)
		{
			if (scene.sceneIndex != 16)
			{
				scene.~Scene();
				new (&scene) Scene(16);
				SetupScenes(16);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 64.0f)
		{
			if (scene.sceneIndex != 17)
			{
				scene.~Scene();
				new (&scene) Scene(17);
				SetupScenes(17);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 69.0f)
		{
			if (scene.sceneIndex != 18)
			{
				scene.~Scene();
				new (&scene) Scene(18);
				SetupScenes(18);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 93.0f)
		{
			if (scene.sceneIndex != 14)
			{
				scene.~Scene();
				new (&scene) Scene(14);
				SetupScenes(14);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 99.0f)
		{
			if (scene.sceneIndex != 20)
			{
				scene.~Scene();
				new (&scene) Scene(20);
				SetupScenes(20);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 104.0f)
		{
			if (scene.sceneIndex != 21)
			{
				scene.~Scene();
				new (&scene) Scene(21);
				SetupScenes(21);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 109.0f)
		{
			if (scene.sceneIndex != 22)
			{
				scene.~Scene();
				new (&scene) Scene(22);
				SetupScenes(22);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 114.0f)
		{
			if (scene.sceneIndex != 23)
			{
				scene.~Scene();
				new (&scene) Scene(23);
				SetupScenes(23);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 120.0f)
		{
			if (scene.sceneIndex != 24)
			{
				scene.~Scene();
				new (&scene) Scene(24);
				SetupScenes(24);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 125.0f)
		{
			if (scene.sceneIndex != 25)
			{
				scene.~Scene();
				new (&scene) Scene(25);
				SetupScenes(25);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 130.5f)
		{
			if (scene.sceneIndex != 26)
			{
				scene.~Scene();
				new (&scene) Scene(26);
				SetupScenes(26);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 136.0f)
		{
			if (scene.sceneIndex != 27)
			{
				scene.~Scene();
				new (&scene) Scene(27);
				SetupScenes(27);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 141.0f)
		{
			if (scene.sceneIndex != 28)
			{
				scene.~Scene();
				new (&scene) Scene(28);
				SetupScenes(28);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 146.0f)
		{
			if (scene.sceneIndex != 29)
			{
				scene.~Scene();
				new (&scene) Scene(29);
				SetupScenes(29);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 151.0f)
		{
			if (scene.sceneIndex != 30)
			{
				scene.~Scene();
				new (&scene) Scene(30);
				SetupScenes(30);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else if (demoTime < 157.0f)
		{
			if (scene.sceneIndex != 19)
			{
				scene.~Scene();
				new (&scene) Scene(19);
				SetupScenes(19);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}
		else
		{
			if (scene.sceneIndex != 8)
			{
				scene.~Scene();
				new (&scene) Scene(8);
				SetupScenes(8);
				animationTime = 0.0f;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
			}
		}

		if (demoTime > 32.0f && demoTime < 37.5f)
		{
			camera.cameraMode = CAMERAMODE::FISHEYE;
		}
		else if (demoTime > 37.5f && demoTime < 43.0f)
		{
			camera.cameraMode = CAMERAMODE::PANINI;
		}
		else
		{
			camera.cameraMode = CAMERAMODE::PINHOLE;
		}
	}

	if (scene.sceneIndex == 8)
	{
		if (playObjectAnimation || playScene || playDemo)
		{
			float3 spherePos = scene.spheres[0].center;
			camera.UpdateSpline(deltaTime / 1000.0f, spherePos);
			if (!physics)
			{
				physics = true;
			}
			changedSetting = true;
		}
	}

	if (scene.sceneIndex == 12)
	{
		if (playObjectAnimation || playScene || playDemo)
		{
			animationTime += deltaTime * 0.001f;
			float rotationSpeed = animationTime * 0.25f;
			scene.voxels[0].SetTransform(mat4::Translate(float3(0.0f, 0.0f, 0.0f)) *
				mat4::Rotate(normalize(float3(1, 1, 1)), rotationSpeed) *
				mat4::Scale(scene.voxels[0].gridScale));
			scene.tlas->Build();

			scene.UpdateSphereSpline(deltaTime * 0.001f);
			scene.bvh->BuildBVH(scene);

			changedSetting = true;
		}

		if (playCameraAnimation || playScene || playDemo)
		{
			float3 cubePos = float3(8 / 512.0f, 8 / 512.0f, 8 / 512.0f);
			camera.UpdateSpline(deltaTime * 0.001f, cubePos);
			changedSetting = true;
		}
	}

	if (scene.sceneIndex == 13)
	{
		if (playObjectAnimation || playScene || playDemo)
		{
			animationTime += deltaTime * 0.001f;
			float rotationSpeed = animationTime * 0.25f;
			float translationTime = animationTime * 0.005f / 4;
			scene.voxels[1].SetTransform(mat4::Translate(float3(0.0075f, 0.01f, 0.005f + translationTime)) *
				mat4::Rotate(normalize(float3(1, 1, 1)), rotationSpeed) *
				mat4::Scale(scene.voxels[1].gridScale));

			scene.tlas->Build();
			changedSetting = true;
		}
		if (playCameraAnimation || playScene || playDemo)
		{
			float3 cubePos = float3(0.0075f, 0.01f, 0.005f + animationTime / 4 * 0.005f);
			camera.UpdateSpline(deltaTime * 0.001f, cubePos);
			changedSetting = true;
		}
	}

	if (scene.sceneIndex == 14)
	{
		if (playObjectAnimation || playScene || playDemo)
		{
			scene.UpdateSphereSpline(deltaTime * 0.001f);
			scene.bvh->BuildBVH(scene);
			changedSetting = true;
		}

		if (playCameraAnimation || playScene || playDemo)
		{
			camera.UpdateSpline(deltaTime * 0.001f, float3(0.2f, 0, -1));
			changedSetting = true;
		}
	}

	if (scene.sceneIndex == 15 || scene.sceneIndex == 16 || scene.sceneIndex == 17 || scene.sceneIndex == 18 || scene.sceneIndex == 19 || scene.sceneIndex == 20 || scene.sceneIndex == 21 || scene.sceneIndex == 22 ||
		scene.sceneIndex == 23 || scene.sceneIndex == 25 || scene.sceneIndex == 26 || scene.sceneIndex == 27 || scene.sceneIndex == 28 || scene.sceneIndex == 29 ||
		scene.sceneIndex == 30)
	{
		if (playCameraAnimation || playScene || playDemo)
		{
			float3 cubePos = float3(32.0f / 512, 5.0f / 512, 32.0f / 512);
			camera.UpdateSpline(deltaTime / 1000.0f, cubePos);
			changedSetting = true;
		}
	}

	if (scene.sceneIndex == 24)
	{
		if (playCameraAnimation || playScene || playDemo)
		{
			float3 cubePos = float3(32.0f / 512, 5.0f / 512, 50.0f / 512);
			camera.UpdateSpline(deltaTime / 1000.0f, cubePos);
		}
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
			float3 average = accumulator[x + y * SCRWIDTH] * scale * fadeFactor;
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

	if (demoTime > 0.0f && demoTime < 10.0f)
	{
		float fullTextStart = 2.0f;
		float fullTextEnd = 8.0f;

		float titleFade = 1.0f;
		if (demoTime < fullTextStart)
		{
			titleFade = demoTime / fullTextStart;
		}
		else if (demoTime > fullTextEnd)
		{
			titleFade = 1.0f - (demoTime - fullTextEnd) / (10.0f - fullTextEnd);
		}

		uint brightnessText = static_cast<uint>(titleFade * 255);
		uint color = (0x00 << 24) | (brightnessText << 16) | (brightnessText << 8) | brightnessText;
		screen->PrintScaled("TAKE ONE", 250, 350, 5, 5, color);
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
			changedSetting |= ImGui::Checkbox("play scene", &playScene);
			changedSetting |= ImGui::Checkbox("play demo", &playDemo);
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
				SetupScenes(13);
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
				SetupScenes(12);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("sphere heart"))
			{
				scene.~Scene();
				new (&scene) Scene(14);
				SetupScenes(14);
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
				SetupScenes(6);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("showcase 1"))
			{
				scene.~Scene();
				new (&scene) Scene(15);
				SetupScenes(15);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("showcase 2"))
			{
				scene.~Scene();
				new (&scene) Scene(16);
				SetupScenes(16);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("showcase 3"))
			{
				scene.~Scene();
				new (&scene) Scene(17);
				SetupScenes(17);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("showcase 4"))
			{
				scene.~Scene();
				new (&scene) Scene(18);
				SetupScenes(18);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("showcase 5"))
			{
				scene.~Scene();
				new (&scene) Scene(19);
				SetupScenes(19);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("showcase 6"))
			{
				scene.~Scene();
				new (&scene) Scene(20);
				SetupScenes(20);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("showcase 7"))
			{
				scene.~Scene();
				new (&scene) Scene(21);
				SetupScenes(21);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("showcase 8"))
			{
				scene.~Scene();
				new (&scene) Scene(22);
				SetupScenes(22);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("showcase 9"))
			{
				scene.~Scene();
				new (&scene) Scene(23);
				SetupScenes(23);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("showcase 10"))
			{
				scene.~Scene();
				new (&scene) Scene(24);
				SetupScenes(24);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("showcase 11"))
			{
				scene.~Scene();
				new (&scene) Scene(25);
				SetupScenes(25);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("showcase 12"))
			{
				scene.~Scene();
				new (&scene) Scene(26);
				SetupScenes(26);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("showcase 13"))
			{
				scene.~Scene();
				new (&scene) Scene(27);
				SetupScenes(27);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("showcase 14"))
			{
				scene.~Scene();
				new (&scene) Scene(28);
				SetupScenes(28);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("showcase 15"))
			{
				scene.~Scene();
				new (&scene) Scene(29);
				SetupScenes(29);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("showcase 16"))
			{
				scene.~Scene();
				new (&scene) Scene(30);
				SetupScenes(30);
				playObjectAnimation = false;
				spp = 1;
				memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float3));
				changedSetting = true;
			}
			if (ImGui::Button("physics"))
			{
				scene.~Scene();
				new (&scene) Scene(8);
				SetupScenes(8);
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
			std::shared_ptr<Hybrid> const hybrid = std::dynamic_pointer_cast<Hybrid>(hybridMat);

			if (hybrid)
			{
				changedSetting |= ImGui::SliderFloat("specular", &hybrid->specular, 0, 1);
				changedSetting |= ImGui::SliderFloat("roughness", &hybrid->roughness, 0, 1);
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("dielectric"))
		{
			std::shared_ptr<Dielectric> const dielectric = std::dynamic_pointer_cast<Dielectric>(dielectricMat);

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

float3 Renderer::SampleSky(const float3& distance) const
{
	float const u = atan2f(distance.z, distance.x) * INV2PI + 0.5f;
	float const v = acosf(distance.y) * INVPI;

	//v = 1.0f - v;

	int iu = static_cast<int>(u * static_cast<float>(skydome->width));
	int iv = static_cast<int>(v * static_cast<float>(skydome->height));

	iu = max(0, min(iu, skydome->width - 1));
	iv = max(0, min(iv, skydome->height - 1));

	uint const pixel = skydome->pixels[iu + iv * skydome->width];

	float const r = ((pixel >> 16) & 255) / 255.0f;
	float const g = ((pixel >> 8) & 255) / 255.0f;
	float const b = (pixel & 255) / 255.0f;
	return float3(r, g, b);
}

float3 Renderer::SampleTexture(float u, float v) const
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

	uint const pixel = texture->pixels[x + y * texture->width];

	float r = ((pixel >> 16) & 255) / 255.0f;
	float g = ((pixel >> 8) & 255) / 255.0f;
	float b = (pixel & 255) / 255.0f;

	return float3(r, g, b);
}


float3 Renderer::Triplanar(float3 const I, float3 const N) const
{
	float constexpr scale = 25.0f;
	float3 const local = I * scale;

	float3 const n = float3(fabs(N.x), fabs(N.y), fabs(N.z));
	float const sum = n.x + n.y + n.z;
	float const bx = n.x / sum;
	float const by = n.y / sum;
	float const bz = n.z / sum;


	//X projection for YZ plane
	float3 const cX = SampleTexture(local.y, local.z);

	//Y projection for XZ plane
	float3 const cY = SampleTexture(local.x, local.z);

	//Z projection for XY plane
	float3 const cZ = SampleTexture(local.x, local.y);

	return cX * bx + cY * by + cZ * bz;
}
