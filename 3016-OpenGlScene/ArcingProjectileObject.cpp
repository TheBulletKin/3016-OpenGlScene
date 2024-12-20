#include "ArcingProjectileObject.h"
#include <cmath>

void ArcingProjectileObject::Launch(vec3 initialVelocity, vec3 initialPosition, float initialTime) {
	this->initialVelocity = initialVelocity;
	this->initialPosition = initialPosition;
	this->currentPosition = initialPosition;
	this->initialTime = initialTime;
	
	this->timeSinceStart = 0.0f;

	
}

void ArcingProjectileObject::UpdatePosition(float deltaTime) {
	timeSinceStart += deltaTime;
	currentPosition.x = initialPosition.x + initialVelocity.x * timeSinceStart;

	currentPosition.z = initialPosition.z + initialVelocity.z * timeSinceStart;

	currentPosition.y = initialPosition.y + initialVelocity.y * timeSinceStart - (0.5 * gravity * pow(timeSinceStart, 2));
	
	
}

bool ArcingProjectileObject::ShouldDestroy() {
	if (currentPosition.y < 0.0f)
	{
		return true;
	}
	return false;
}
