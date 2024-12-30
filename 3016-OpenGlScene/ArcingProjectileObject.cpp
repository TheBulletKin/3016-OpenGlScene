#include "ArcingProjectileObject.h"
#include <cmath>

void ArcingProjectileObject::Launch(vec3 initialVelocity, vec3 initialPosition, float initialTime, float gravityMultiplier, float movespeedMultiplier) {
	this->initialVelocity = initialVelocity;
	this->initialPosition = initialPosition;
	this->currentPosition = initialPosition;
	this->initialTime = initialTime;
	this->gravityMultiplier = gravityMultiplier;
	this->movespeedMultiplier = movespeedMultiplier;
	
	this->timeSinceStart = 0.0f;
	this->lifetimeMax = 20.0f;
	
}

void ArcingProjectileObject::UpdatePosition(float deltaTime) {
	timeSinceStart += deltaTime;
	currentPosition.x = initialPosition.x + initialVelocity.x * (timeSinceStart * movespeedMultiplier);

	currentPosition.z = initialPosition.z + initialVelocity.z * (timeSinceStart * movespeedMultiplier);

	currentPosition.y = initialPosition.y + initialVelocity.y * (timeSinceStart - (0.5 * (gravity * gravityMultiplier) * pow(timeSinceStart, 2)) * movespeedMultiplier);
	
	
}

bool ArcingProjectileObject::ShouldDestroy() {
	if (currentPosition.y < 0.0f || timeSinceStart >= lifetimeMax)
	{
		return true;
	}
	return false;
}
