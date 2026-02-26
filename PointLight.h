#pragma once

#include "Lighting.h"

class PointLight : public Lighting
{
public:
	PointLight(const float3& pos, const float3& color);

	float3 SampleDirection(const float3& point) const override;
	float3 Radiance(const float3& point) const override;
	bool IsOccluded(const float3& point, const Scene& scene) const override;
private:
	float3 pos;
	float3 color;
};

