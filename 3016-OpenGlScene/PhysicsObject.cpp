#include "PhysicsObject.h"
#include <cmath>

void PhysicsObject::Launch(vec3 initialVelocity, vec3 initialPosition, float initialTime) {
	this->initialVelocity = initialVelocity;
	this->initialPosition = initialPosition;
	this->currentPosition = initialPosition;
	this->initialTime = initialTime;
	this->currentTime = 0.0f;
}

void PhysicsObject::UpdatePosition(float deltaTime) {
	currentTime += deltaTime;
	currentPosition.x = initialPosition.x + initialVelocity.x * currentTime;

	currentPosition.z = initialPosition.z + initialVelocity.z * currentTime;

	currentPosition.y = initialPosition.y + initialVelocity.y * currentTime - (0.5 * gravity * pow(currentTime, 2));
	if (currentPosition.y < 0.0f) {
		currentPosition.y = 0.0f;		
	}
}
