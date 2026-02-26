#include "template.h"
#include "DirectionalLight.h"


DirectionalLight::DirectionalLight(const float3& direction, const float3& color)
	: direction(direction), color(color)
{
}

float3 DirectionalLight::SampleDirection(const float3& [[maybe_unused]]point) const
{
	return -direction;
}

float3 DirectionalLight::Radiance(const float3& [[maybe_unused]]point) const
{
	return color;
}

bool DirectionalLight::IsOccluded(const float3& point, const Scene& scene) const
{
	float3 dir = -direction;

	Ray shadowRay(point, dir);
	return scene.IsOccluded(shadowRay);
}
