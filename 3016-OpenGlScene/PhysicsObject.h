#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "CustomSceneObject.h"

using namespace glm;

class PhysicsObject : public CustomSceneObject   
{
public:
	PhysicsObject() : CustomSceneObject() {};
	~PhysicsObject() {};
	void Launch(vec3 initialVelocity, vec3 initialPosition, float initialTime);
	bool UpdatePosition(float deltaTime);

	vec3 initialVelocity;
	vec3 initialPosition;
	vec3 currentPosition;
	float initialTime;
	float currentTime;
	float gravity = 9.81f;
private:
	
};

