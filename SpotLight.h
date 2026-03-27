#pragma once

#include "Lighting.h"

class SpotLight : public Lighting
{
public:
	SpotLight(const float3& pos, const float3& direction, const float3& color, float innerCutOff, float outerCutOff);

	float3 SampleDirection(const float3& point) const;
	float3 Radiance(const float3& point) const;
	bool IsOccluded(const float3& point, const Scene& scene) const;
private:
	float3 pos;
	float3 direction;
	float3 color;
	float innerCutOff;
	float outerCutOff;
};

