#pragma once

#include "Lighting.h"
#include "AreaLight.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "RollBall.h"
#include "SpotLight.h"
#include "Audio.h"
//#include "Material.h"

class Material;

namespace Tmpl8
{

class Renderer : public TheApp
{
public:
	// game flow methods
	void Init();
	float3 Trace( Ray& ray, int = 0, int = 0, int = 0 );
	float3 Shade(const float3& N, const float3& I);
	void Tick( float deltaTime );
	void UI();
	float3 SampleSky(float3& distance);
	float3 SampleTexture(float u, float v);
	float3 Triplanar(float3 I, float3 N);
	void Shutdown() { /* nothing here for now */ }
	// input handling
	void MouseUp( int button ) { button = 0; /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { button = 0; /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y )
	{
	#if defined(DOUBLESIZE) && !defined(FULLSCREEN)
		mousePos.x = x / 2, mousePos.y = y / 2;
	#else
		mousePos.x = x, mousePos.y = y;
	#endif
	}
	void MouseWheel( float y ) { y = 0; /* implement if you want to handle the mouse wheel */ }
	void KeyUp( int key ) { key = 0; /* implement if you want to handle keys */ }
	void KeyDown( int key ) { key = 0; /* implement if you want to handle keys */ }
	// data members
	int2 mousePos;
	float3* accumulator;	// for episode 3
	float3* history;		// for episode 5
	Scene scene;
	Camera camera;
	RollBall rollBall;
	Audio audio;

	bool changedSetting = false;
	float cameraTime = 0.0f;
	bool physics = false;
	bool playCameraAnimation = false;
	bool playObjectAnimation = false;

	float fps;
	int maxDepth = 1;
	float specular = 1.0f;
	float roughness = 0.02f;
	DirectionalLight* directionalLight;
	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
	std::vector<AreaLight*> areaLights;

	std::shared_ptr<Material> reflectiveMat;
	std::shared_ptr<Material> refractiveMat;
	std::shared_ptr<Material> dielectricMat;
	std::shared_ptr<Material> hybridMat;

	Surface* texture;
	Surface* skydome;

	bool pinhole = false;
	bool panini = false;
	bool fishEye = false;
	bool aperture = false;
};

} // namespace Tmpl8