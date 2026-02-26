#pragma once

#include "Material.h"

class Hybrid : public Material
{
public:
	Hybrid(float specular, float roughness);
	float3 calc(Renderer& renderer, const Ray& ray, const float3 N, float3 I, const int depth) const override;
	
	float specular;
	float roughness;
private:

};

