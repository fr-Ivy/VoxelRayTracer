#pragma once
class RollBall
{
public:
	void Move(float deltaTime, Sphere& ball, Scene& scene);
	bool hit(Sphere& sphere, Scene& scene);

private:
};

