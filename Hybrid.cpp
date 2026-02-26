#include "template.h"
#include "Hybrid.h"

Hybrid::Hybrid(float specular, float roughness)
	: specular(specular), roughness(roughness)
{
}

float3 Hybrid::calc(Renderer& renderer, const Ray& ray, const float3 N, float3 I, const int depth) const
{
	float3 R = ray.D - float3(2) * N * dot(ray.D, N);

	float3 randomInUnitSphere = float3(RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f);
	float3 randomDir = normalize(randomInUnitSphere) * roughness;
	float3 blurryRay = normalize(R + randomDir);

	Ray aRay = Ray(I, blurryRay);
	float3 reflected = renderer.Trace(aRay, depth + 1);

	return (1.0f - specular) * float3(1.0f) + specular * reflected;
}
