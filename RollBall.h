#pragma once
class RollBall
{
public:
	void Move(float deltaTime, Sphere& ball, Scene& scene);
	void hit(Sphere& ball, Scene& scene);

private:
};

