#pragma once

#include "Lighting.h"

class DirectionalLight : public Lighting
{
public:
	DirectionalLight(const float3& direction, const float3& color);

	float3 SampleDirection(const float3& point) const override;
	float3 Radiance(const float3& point) const override;
	bool IsOccluded(const float3& point, const Scene& scene) const override;
	float3 direction;
	float3 color;
private:
};

