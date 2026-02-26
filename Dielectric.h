#pragma once

#include "Material.h"

class Dielectric : public Material
{
public:
	float3 calc(Renderer& renderer, const Ray& ray, const float3 N, float3 I, const int depth) const override;
	float index1 = 1.00f;
	float index2 = 1.46f;
};

