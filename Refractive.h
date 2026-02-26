#pragma once

#include "Material.h"

class Refractive : public Material
{
public:
	float3 calc(Renderer& renderer, const Ray& ray, const float3 N, float3 I, const int depth) const override;

private:
	
};

