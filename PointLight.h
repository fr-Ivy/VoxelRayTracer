#pragma once

class PointLight
{
public:
	PointLight(const float3& pos, const float3& color);

	float3 SampleDirection(const float3& point) const;
	float3 Radiance(const float3& point) const;
	bool IsOccluded(const float3& point, const Scene& scene) const;
private:
	float3 pos;
	float3 color;
};

