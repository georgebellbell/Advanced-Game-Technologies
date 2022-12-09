#include "Powerup.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "Transform.h"
#include "CollisionVolume.h"


using namespace NCL;
using namespace CSC8503;

Powerup::Powerup(const Vector3& position, PowerupType type)
{
	name = "powerup";
	this->type = type;

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	SetBoundingVolume((CollisionVolume*)volume);
	volume->SetDiscrete(false);

	transform
		.SetScale(sphereSize)
		.SetPosition(position);

	SetPhysicsObject(new PhysicsObject(&transform, GetBoundingVolume()));
	physicsObject->InitSphereInertia();
	physicsObject->SetElasticity(0.2);
	physicsObject->SetInverseMass(inverseMass);

	boundingVolume->SetTrigger(true);

	objectLayer = powerups;
	objectLayerMask = powerupLayerMask;
}
