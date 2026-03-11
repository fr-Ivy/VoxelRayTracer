#include "template.h"
#include "PointLight.h"


PointLight::PointLight(const float3& pos, const float3& color)
	: pos(pos), color(color)
{
}

float3 PointLight::SampleDirection(const float3& point) const
{
	float3 L = pos - point;
	return normalize(L);
}

float3 PointLight::Radiance(const float3& point) const
{
	float3 L = pos - point;
	float distance = length(L);

	// Attenuation based on the inverse square formula.
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	float attenuation = 1 / (constant + linear * distance + quadratic * (distance * distance));

	return color * attenuation;
}

bool PointLight::IsOccluded(const float3& point, const Scene& scene) const
{
	float3 L = pos - point;
	float distance = length(L);

	Ray shadowRay(point, L, distance);
	return scene.IsOccluded(shadowRay);
}
