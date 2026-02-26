#pragma once

class Lighting
{
public:
	virtual ~Lighting() = default;

	virtual float3 SampleDirection(const float3& point) const = 0;

	virtual float3 Radiance(const float3& point) const = 0;

	virtual bool IsOccluded(const float3& point, const Scene& scene) const = 0;

private:
};