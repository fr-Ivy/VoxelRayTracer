#pragma once

class AreaLight
{
public:
	virtual bool sample(const float3& intersectionPoint, const float2& randomNumber, float3& direction, float3& color, float& distance, float& pdf) const = 0;

	virtual float3 getNormal() const = 0;
};