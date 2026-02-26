#include "template.h"
#include "SpotLight.h"


SpotLight::SpotLight(const float3& pos, const float3& direction, const float3& color, float innerCutOff, float outerCutOff)
	: pos(pos), direction(direction), color(color), innerCutOff(innerCutOff), outerCutOff(outerCutOff)
{
}

float3 SpotLight::SampleDirection(const float3& point) const
{
	float3 L = pos - point;
	return normalize(L);
}

float3 SpotLight::Radiance(const float3& point) const
{
	float3 lightDir = normalize(-direction);
	float3 L = pos - point;
	float distance = length(L);

	float spotFactor = dot(normalize(L), lightDir);

	float constant = 1.0f;
	float linear = 0.15f;
	float quadratic = 0.1f;

	float attenuation = 1 / (constant + linear * distance + quadratic * (distance * distance));

	float inner = std::cos(innerCutOff * PI / 180);
	float outer = std::cos(outerCutOff * PI / 180);

	float spot = (spotFactor - outer) / (inner - outer);
	spot = max(0.0f, min(1.0f, spot));

	if (spot <= 0.0f)
	{
		return float3(0, 0, 0);
	}

	return color * attenuation * spot;
}

bool SpotLight::IsOccluded(const float3& point, const Scene& scene) const
{
	float3 L = pos - point;
	float distance = length(L);

	Ray shadowRay(point, L, distance);
	return scene.IsOccluded(shadowRay);
}
