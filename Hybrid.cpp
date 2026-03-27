#include "template.h"
#include "Hybrid.h"

Hybrid::Hybrid(float specular, float roughness)
	: specular(specular), roughness(roughness)
{
}

float3 Hybrid::calc(Renderer& renderer, const Ray& ray, const float3 N, float3 I, const int depth) const
{
	// Calculate roughness and specular for the reflection ray.
	float3 R = ray.D - float3(2) * N * dot(ray.D, N);

	float3 blurryRay;
	if (roughness >= 0.001f)
	{
		float3 randomInUnitSphere = float3(RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f);
		float3 randomDir = normalize(randomInUnitSphere) * roughness;
		blurryRay = normalize(R + randomDir);
	}
	else
	{
		blurryRay = R;
	}

	Ray aRay = Ray(I, blurryRay);
	float3 reflected = renderer.Trace(aRay, depth + 1);

	return reflected;
	//return (1.0f - specular) * float3(1.0f) + specular * reflected;
}
