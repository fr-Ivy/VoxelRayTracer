#pragma once

#include "AreaLight.h"

class QuadLight : public AreaLight
{
public:
	QuadLight(const float3& pos, const float3& width, const float3& height, const float3& color);

	bool Sample(const float3& intersectionPoint, const float2& randomNumber, float3& direction, float3& color, float& distance, float& pdf) const override;

	float3 GetNormal() const override;
private:
	float3 pos;
	float3 width;
	float3 height;
	float3 normal;
	float3 color;
	float area;
};

