#include "template.h"
#include "Refractive.h"

float3 Refractive::Calc(Renderer& renderer, const Ray& ray, const float3 N, float3 const I, const int depth) const
{
	// Calculate the refraction ray direction using Snell's law.
	float n1 = 1.0f, n2 = 1.46f;

	if (ray.inside)
	{
		swap(n1, n2);
	}

	float d = dot(N, -ray.D);
	float eta = n1 / n2;
	float k = 1 - eta * eta * (1 - pow2f(d));


	if (k < 0)
	{
		return float3(0);
	}

	float3 R = eta * ray.D + (eta * d - sqrt(k)) * N;
	Ray aRay = Ray(I, R);
	return renderer.Trace(aRay, depth + 1);
}
