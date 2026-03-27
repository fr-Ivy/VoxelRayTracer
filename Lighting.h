#pragma once

class Lighting
{
public:
	~Lighting() = default;

	float3 SampleDirection(const float3& point) const;

	float3 Radiance(const float3& point) const;

	bool IsOccluded(const float3& point, const Scene& scene) const;

private:
};