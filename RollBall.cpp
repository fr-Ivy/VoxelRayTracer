#include "template.h"
#include "RollBall.h"

void RollBall::Move(float deltaTime, Sphere& ball, Scene& scene)
{
	deltaTime /= 1000.0f;

	//std::cout << ball.center.x << ", " << ball.center.y << ", " << ball.center.z << std::endl;

	// gravity
	float3 gravity = float3(0, -9.81f / WORLDSIZE, 0);

	ball.velocity = ball.center - ball.previousCenter;


	float3 center = ball.center + ball.velocity + gravity * deltaTime * deltaTime;
	ball.previousCenter = ball.center;
	ball.center = center;

	hit(ball, scene);

	ball.velocity = ball.center - ball.previousCenter;

	ball.velocity.y = min(0.0f, ball.velocity.y);

	ball.previousCenter = ball.center - ball.velocity;
}

void RollBall::hit(Sphere& ball, Scene& scene)
{
	float3 directions[26] = 
	{
		float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1),
		float3(-1, 0, 0), float3(0, -1, 0), float3(0, 0, -1),

		float3(1, 1, 0), float3(0, 1, 1), float3(1, 0, 1),
		float3(-1, -1, 0), float3(0, -1, -1), float3(-1, 0, -1),
		float3(-1, 1, 0), float3(0, -1, 1), float3(-1, 0, 1),
		float3(1, -1, 0), float3(0, 1, -1), float3(1, 0, -1),

		float3(1, 1, 1), float3(1, 1, -1), float3(1, -1, 1),
		float3(1, -1, -1), float3(-1, 1, 1), float3(-1, 1, -1),
		float3(-1, -1, 1), float3(-1, -1, -1)
	};

	for (int j = 0; j < 6; j++)
	{
		for (int i = 0; i < 26; i++)
		{
			float3 direction = normalize(directions[i]);
			Ray sphereRay(ball.center, direction, ball.radius);
			scene.FindNearest(sphereRay, true);

			// if the ray hits something that's closer that the balls radius: collision
			if (sphereRay.t < ball.radius && sphereRay.t > 0.0f && !sphereRay.hitSphere)
			{
				//std::cout << "hit something" << std::endl;
				float penetration = ball.radius - sphereRay.t;
				penetration = max(0.0f, min(penetration, ball.radius));

				float3 push = -direction * penetration;
				ball.center += push;
			}
		}
	}
}

