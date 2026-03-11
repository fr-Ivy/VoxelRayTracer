#include "template.h"
#include "RollBall.h"

void RollBall::Move(float deltaTime, Sphere& ball, Scene& scene)
{
	deltaTime /= 10000.0f;
	ball.velocity.y -= 9.81f * deltaTime;


	Ray down(ball.center, float3(0, -1, 0));

	ball.center += ball.velocity * deltaTime;
}

bool RollBall::hit(Sphere& sphere, Scene& scene)
{
	
	return false;
}
