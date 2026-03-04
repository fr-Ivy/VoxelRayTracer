#include "template.h"

Camera::Camera()
{
	// try to load a camera
	FILE* f = fopen( "camera.bin", "rb" );
	if (f)
	{
		fread( this, 1, sizeof( Camera ), f );
		fclose( f );

		//fov = 30.0f;
		//paniniStrength = 1.0f;
		//apertureRadius = 0.003f;
		//focusDistance = 0.2f;
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
	const float u = static_cast<float>(x) * (1.0f / SCRWIDTH);
	const float v = static_cast<float>(y) * (1.0f / SCRHEIGHT);
	const float3 P = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);
	// return Ray( camPos, normalize( P - camPos ) );

	float3 front = normalize(camTarget - camPos);
	float3 right = normalize(cross(float3(0, 1, 0), front));
	float3 up = normalize(cross(front, right));

	float3 dir = normalize(P - camPos);
	float forward = dot(dir, front);


	float3 focusPoint;
	float2 tc;

	switch (cameraMode)
	{
	case CAMERAMODE::PINHOLE:
		focusPoint = camPos + dir * focusDistance;
		break;
	case CAMERAMODE::PANINI:
		tc = float2(dot(dir, right) / forward, dot(dir, up) / forward);
		float3 panini = PaniniProjection(tc, fov, paniniStrength);
		float3 worldDirPanini = panini.x * right + panini.y * up + panini.z * front;
		focusPoint = camPos + worldDirPanini * focusDistance;
		break;
	case CAMERAMODE::FISHEYE:
		float3 fisheye = FishEyeLens(dir, right, up, front, fov);
		float3 worldDirFishEye = fisheye.x * right + fisheye.y * up + fisheye.z * front;
		focusPoint = camPos + worldDirFishEye * focusDistance;
	}

	float2 randomInUnitSquare = float2(RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f);
	float randomDistance = pow2f(randomInUnitSquare.x) + pow2f(randomInUnitSquare.y);

	do
	{
		randomInUnitSquare = float2(RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f);
		randomDistance = pow2f(randomInUnitSquare.x) + pow2f(randomInUnitSquare.y);
		//std::cout << randomDistance << std::endl;
	} while(randomDistance > 1);

	float3 lensOffset = (randomInUnitSquare.x * right + randomInUnitSquare.y * up) * apertureRadius;
	//float3 lensOffset = float3(0);

	float3 rayOrigin = camPos + lensOffset;
	float3 rayDir = focusPoint - rayOrigin;

	//return Ray(camPos, P - camPos);
	return Ray(rayOrigin, rayDir);
	//return Ray(camPos, panini.x * right + panini.y * up + panini.z * front);

	// Note: no need to normalize primary rays in a pure voxel world
	// TODO: 
	// - if we have other primitives as well, we *do* need to normalize!
}

float3 Camera::PaniniProjection(float2& tc, float fov_degrees, float d)
{
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
	return true;
}
