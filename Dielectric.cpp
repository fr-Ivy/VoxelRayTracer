#include "template.h"
#include "Dielectric.h"

float3 Dielectric::calc(Renderer& renderer, const Ray& ray, const float3 N, float3 I, const int depth) const
{
	float n1 = index1, n2 = index2;

	if (ray.inside)
	{
		swap(n1, n2);
	}

	float d = dot(N, -ray.D);
	float eta = n1 / n2;
	float k = 1 - eta * eta * (1 - pow2f(d));

	float3 reflected(0), refracted(0);

	if (k > 0)
	{
		float3 R = eta * ray.D + (eta * d - sqrt(k)) * N;
		Ray aRay = Ray(I, R);
		refracted = renderer.Trace(aRay, depth + 1);
	}

	float3 R = ray.D - float3(2) * N * d;
	Ray aRay = Ray(I, R);
	reflected = renderer.Trace(aRay, depth + 1);

	float R0 = pow2f((n1 - n2) / (n1 + n2));
	float fresnel = k > 0 ? R0 + (1 - R0) * pow5f(1 - d) : 1;
	return fresnel * reflected + (1 - fresnel) * refracted;
}
