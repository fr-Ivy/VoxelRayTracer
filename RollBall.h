#pragma once
class RollBall
{
public:
	void Move(float deltaTime, Sphere& ball, Scene& scene);
	void Hit(Sphere& ball, Scene& scene);

private:
};

