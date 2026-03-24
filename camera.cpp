#include "template.h"

Camera::Camera()
{
	// try to load a camera
	FILE* f = fopen( "camera.bin", "rb" );
	if (f)
	{
		fread( this, 1, sizeof( Camera ), f );
		fclose( f );
	}
	else
	{
		// setup a basic view frustum
		camPos = float3( 0, 0, -2 );
		camTarget = float3( 0, 0, -1 );
		topLeft = float3( -aspect, 1, 0 );
		topRight = float3( aspect, 1, 0 );
		bottomLeft = float3( -aspect, -1, 0 );
	}
}

Camera::~Camera()
{
	// save current camera
	FILE* f = fopen( "camera.bin", "wb" );
	fwrite( this, 1, sizeof( Camera ), f );
	fclose( f );
}

Ray Camera::GetPrimaryRay( const float x, const float y )
{
	// calculate pixel position on virtual screen plane
	float u = x / SCRWIDTH;
	float v = y / SCRHEIGHT;

	//float3 directionCamera = normalize(float3(u * aspect, -v, -1));

	const float3 P = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);
	// return Ray( camPos, normalize( P - camPos ) );

	// get world space coordinates of the camera.
	float3 right = float3(cameraToWorld(0, 0), cameraToWorld(1, 0), cameraToWorld(2, 0));
	float3 up = float3(cameraToWorld(0, 1), cameraToWorld(1, 1), cameraToWorld(2, 1));
	float3 front = float3(cameraToWorld(0, 2), cameraToWorld(1, 2), cameraToWorld(2, 2));

	float3 dir = normalize(P - camPos);
	float3 directionWorld = normalize(dir.x * right + dir.y * up + dir.z * front);

	float3 cameraWorldPos = float3(cameraToWorld(0, 3), cameraToWorld(1, 3), cameraToWorld(2, 3));

	float3 finalDirection = dir;

	switch (cameraMode)
	{
	case CAMERAMODE::PINHOLE:
		break;
	case CAMERAMODE::PANINI:
	{
		float forward = dot(dir, front);

		float2 tc = float2(dot(dir, right) / forward, dot(dir, up) / forward);
		float3 panini = PaniniProjection(tc, fov, paniniStrength);
		finalDirection = normalize(panini.x * right + panini.y * up + panini.z * front);
		break;
	}
	case CAMERAMODE::FISHEYE:
		float3 fisheye = FishEyeLens(dir, right, up, front, fov);
		finalDirection = normalize(fisheye.x * right + fisheye.y * up + fisheye.z * front);
	}

	// sample a random point on the lens
	float2 randomInUnitSquare = float2(RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f);
	float randomDistance = pow2f(randomInUnitSquare.x) + pow2f(randomInUnitSquare.y);

	// keep sampling until we get a point inside the unit circle.
	do
	{
		randomInUnitSquare = float2(RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f);
		randomDistance = pow2f(randomInUnitSquare.x) + pow2f(randomInUnitSquare.y);
		//std::cout << randomDistance << std::endl;
	} while(randomDistance > 1);

	// scale the random point to the aperture size and calculate the ray origin and direction
	float3 lensOffset = (randomInUnitSquare.x * right + randomInUnitSquare.y * up) * apertureRadius;
	//float3 lensOffset = float3(0);

	float3 rayOrigin = cameraWorldPos + lensOffset;
	float3 focusPoint = cameraWorldPos + finalDirection * focusDistance;
	float3 rayDir = normalize(focusPoint - rayOrigin);

	//return Ray(camPos, P - camPos);
	return Ray(rayOrigin, rayDir);
	//return Ray(camPos, panini.x * right + panini.y * up + panini.z * front);

	// Note: no need to normalize primary rays in a pure voxel world
	// TODO: 
	// - if we have other primitives as well, we *do* need to normalize!
}

float3 Camera::PaniniProjection(float2& tc, float fov_degrees, float d)
{
	// based on https://www.shadertoy.com/view/Wt3fzB
	float fov_radians = fov_degrees * (PI / 180);

	float d2 = pow2f(d);

	float fo = PI / 2 - fov_radians * 0.5f;

	float f = cos(fo) / sin(fo) * 2.0f;
	float f2 = pow2f(f);

	float b = (sqrt(max(0.0f, pow2f(d + d2) * (f2 + pow2f(f2)))) - (d * f + f)) / (d2 + d2 * f2 - 1.0f);

	//printf("b = %f, f = %f\n", b, f);

	tc *= b;

	float h = tc.x;
	float v = tc.y;

	float h2 = pow2f(h);

	float k = h2 / pow2f(d + 1.0f);
	float k2 = pow2f(k);

	float discr = max(0.0f, k2 * d2 - (k + 1.0f) * (k * d2 - 1.0f));

	float cosPhi = (-k * d + sqrt(discr)) / (k + 1.0f);
	float S = (d + 1.0f) / (d + cosPhi);
	float tanTheta = v / S;

	float sinPhi = sqrt(max(0.0f, 1.0f - pow2f(cosPhi)));

	if (tc.x < 0.0f)
	{
		sinPhi *= -1.0f;
	}

	float s = 1.0f / sqrt(1.0f + pow2f(tanTheta));

	return float3(sinPhi, tanTheta, cosPhi) * s;
}

float3 Camera::FishEyeLens(float3 dir, float3 right, float3 up, float3 front, float fov_degrees)
{
	// calculate the fisheye direction based on cameras world coordinates.
	float fov_radians = fov_degrees * (PI / 180);

	//float distance = sqrt(pow2f(tc.x) + pow2f(tc.y));
	float theta = acos(dot(dir, front));
	float phi = atan2(dot(dir, up), dot(dir, right));
	float maxTheta = fov_radians * 0.5f;
	theta = theta / maxTheta * (PI * 0.5f);

	float3 local = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
	return local;
}

bool Camera::HandleInput( const float t )
{
	if (!WindowHasFocus()) return false;
	float speed = 0.0005f * t;
	float3 ahead = normalize( camTarget - camPos );
	float3 tmpUp( 0, 1, 0 );
	float3 right = normalize( cross( tmpUp, ahead ) );
	float3 up = normalize( cross( ahead, right ) );
	bool changed = false;
	if (IsKeyDown( GLFW_KEY_UP )) camTarget -= speed * up, changed = true;
	if (IsKeyDown( GLFW_KEY_DOWN )) camTarget += speed * up, changed = true;
	if (IsKeyDown( GLFW_KEY_LEFT )) camTarget -= speed * right, changed = true;
	if (IsKeyDown( GLFW_KEY_RIGHT )) camTarget += speed * right, changed = true;
	ahead = normalize( camTarget - camPos );
	right = normalize( cross( tmpUp, ahead ) );
	up = normalize( cross( ahead, right ) );
	if (IsKeyDown( GLFW_KEY_A )) camPos -= speed * right, changed = true;
	if (IsKeyDown( GLFW_KEY_D )) camPos += speed * right, changed = true;
	if (GetAsyncKeyState( 'W' )) camPos += speed * ahead, changed = true;
	if (IsKeyDown( GLFW_KEY_S )) camPos -= speed * ahead, changed = true;
	if (IsKeyDown( GLFW_KEY_R )) camPos += speed * up, changed = true;
	if (IsKeyDown( GLFW_KEY_F )) camPos -= speed * up, changed = true;
	camTarget = camPos + ahead;
	ahead = normalize( camTarget - camPos );
	up = normalize( cross( ahead, right ) );
	right = normalize( cross( up, ahead ) );
	topLeft = camPos + 2.0f * ahead - aspect * right + up;
	topRight = camPos + 2.0f * ahead + aspect * right + up;
	bottomLeft = camPos + 2.0f * ahead - aspect * right - up;
	if (!changed) return false;
	LookAt();
	return true;
}

void Camera::CatmullRomSplines(float3 p0, float3 p1, float3 p2, float3 p3, float t)
{
	float alpha = 0.5f;
	float tension = 0.0f;

	float t01 = pow(length(p1 - p0), alpha);
	float t12 = pow(length(p2 - p1), alpha);
	float t23 = pow(length(p3 - p2), alpha);

	float3 m1 = (1.0f - tension) *
		(p2 - p1 + t12 * ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)));
	float3 m2 = (1.0f - tension) *
		(p2 - p1 + t12 * ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)));

	SEGMENT segment;
	segment.a = 2.0f * (p1 - p2) + m1 + m2;
	segment.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
	segment.c = m1;
	segment.d = p1;

	camPos = segment.a * t * t * t + segment.b * t * t + segment.c * t + segment.d;
	camTarget = float3(0.5f, 0.3f, 0.5f);

	LookAt();
	
}

void Camera::LookAt()
{
	worldToCamera = mat4::LookAt(camPos, camTarget, float3(0, 1, 0));
	cameraToWorld = worldToCamera.Inverted();
}

