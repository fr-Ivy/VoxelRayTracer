#include "template.h"
#include "Reflective.h"

float3 Reflective::Calc(Renderer& renderer, const Ray& ray, const float3 N, float3 const I, const int depth) const
{
	// Calculate the reflection ray direction.
	float3 R = ray.D - float3(2) * N * dot(ray.D, N);
	Ray aRay = Ray(I, R);
	return renderer.Trace(aRay, depth + 1);
}
