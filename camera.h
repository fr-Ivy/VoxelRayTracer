#pragma once

// default screen resolution
#define SCRWIDTH	960 //640 or 960 (or 800 - not recommended)
#define SCRHEIGHT	600 //400 or 600
// #define FULLSCREEN
#define DOUBLESIZE

namespace Tmpl8 {

	enum class CAMERAMODE
	{
		PINHOLE,
		PANINI,
		FISHEYE
	};

	struct SEGMENT
	{
		float3 a;
		float3 b;
		float3 c;
		float3 d;
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
	void CatmullRomSplines(float3 p0, float3 p1, float3 p2, float3 p3, float t);
	void LookAt();
	float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
	float3 camPos, camTarget;
	mat4 cameraToWorld;
	mat4 worldToCamera;

	float3 topLeft, topRight, bottomLeft;
	float fov = 30.0f;
	float paniniStrength = 1.0f;
	float focusDistance = 0.2f;
	float apertureRadius = 0.003f;
	CAMERAMODE cameraMode = CAMERAMODE::PINHOLE;
	bool aperture = false;
};

}