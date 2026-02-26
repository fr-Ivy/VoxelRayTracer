#include "template.h"
#include "QuadLight.h"


QuadLight::QuadLight(const float3& pos, const float3& width, const float3& height, const float3& color)
	: pos(pos), width(width), height(height), color(color)
{
	normal = normalize(cross(width, height));
	if (normal.y > 0)
	{
		normal = -normal;
	}

	area = length(cross(width, height));
}

bool QuadLight::sample(const float3& intersectionPoint, const float2& randomNumber, float3& direction, float3& emission,
                       float& distance, float& pdf) const
{
	float3 d = (pos + width * randomNumber.x + height * randomNumber.y) - intersectionPoint;
	distance = length(d);

	direction = d / distance;

	float d_dot_N = dot(d, normal);
	if (d_dot_N >= 0)
	{
		return false;
	}

	pdf = 1.0f / area;
	emission = color;

	return true;
}

float3 QuadLight::getNormal() const
{
	return normal;
}
