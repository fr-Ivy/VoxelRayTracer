#include "template.h"
#include "QuadLight.h"


QuadLight::QuadLight(const float3& pos, const float3& width, const float3& height, const float3& color)
	: pos(pos), width(width), height(height), color(color)
{
	// Calculate the normal of the quad light using the cross product of the width and the height of the quad light.
	normal = normalize(cross(width, height));

	// Calculate the area of the quad light using the length of the cross product of the width and the height of the quad light.
	area = length(cross(width, height));
}

bool QuadLight::Sample(const float3& intersectionPoint, const float2& randomNumber, float3& direction, float3& emission,
                       float& distance, float& pdf) const
{
	// Sample a point on the quad light using random numbers.
	float3 const d = (pos + width * randomNumber.x + height * randomNumber.y) - intersectionPoint;
	distance = length(d);

	direction = d / distance;

	// Calculate the dot product of the direction and the normal to determine if the light is facing towards the intersection point.
	float d_dot_N = dot(d, normal);
	if (d_dot_N >= 0)
	{
		return false;
	}

	// Calculate the PDF for the sampled point on the quad light.
	pdf = 1.0f / area;
	emission = color;

	return true;
}

float3 QuadLight::GetNormal() const
{
	return normal;
}
