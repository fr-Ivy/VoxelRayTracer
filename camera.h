#pragma once

// default screen resolution
#define SCRWIDTH	640 //640
#define SCRHEIGHT	400 //400
// #define FULLSCREEN
#define DOUBLESIZE

namespace Tmpl8 {

	enum class CAMERAMODE
	{
		PINHOLE,
		PANINI,
		FISHEYE
	};

class Camera
{
public:
	Camera();
	~Camera();
	Ray GetPrimaryRay( const float x, const float y );
	float3 PaniniProjection(float2& tc, float fov_degrees, float d);
	float3 FishEyeLens(float3 dir, float3 right, float3 up, float3 front, float fov_degrees);
	bool HandleInput( const float t );
	float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
	float3 camPos, camTarget;
	float3 topLeft, topRight, bottomLeft;
	float fov = 30.0f;
	float paniniStrength = 1.0f;
	float focusDistance = 0.2f;
	float apertureRadius = 0.003f;
	CAMERAMODE cameraMode = CAMERAMODE::PINHOLE;
	bool aperture = false;
};

}