#pragma once
class Material
{
public:
	virtual float3 calc(Renderer& renderer, const Ray& ray, const float3 N, float3 I, const int depth) const = 0;
private:

};

