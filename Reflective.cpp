#include "template.h"
#include "Reflective.h"

float3 Reflective::calc(Renderer& renderer, const Ray& ray, const float3 N, float3 I, const int depth) const
{
	float3 R = ray.D - float3(2) * N * dot(ray.D, N);
	Ray aRay = Ray(I, R);
	return renderer.Trace(aRay, depth + 1);
}
