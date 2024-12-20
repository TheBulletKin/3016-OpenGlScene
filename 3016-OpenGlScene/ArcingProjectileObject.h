#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "CustomSceneObject.h"

using namespace glm;

class ArcingProjectileObject : public CustomSceneObject   
{
public:
	ArcingProjectileObject() : CustomSceneObject() {};
	~ArcingProjectileObject() {};
	void Launch(vec3 initialVelocity, vec3 initialPosition, float initialTime);
	void UpdatePosition(float deltaTime);
	bool ShouldDestroy();

	vec3 initialVelocity;
	vec3 initialPosition;
	vec3 currentPosition;
	float initialTime;
	float timeSinceStart;
	float gravity = 9.81f;
private:
	
};

