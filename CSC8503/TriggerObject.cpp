#include "TriggerObject.h"
#include "PhysicsObject.h"

TriggerObject::TriggerObject(const Vector3& position, Vector3 dimensions)
{
	AABBVolume* volume = new AABBVolume(dimensions);
	SetBoundingVolume((CollisionVolume*)volume);

	boundingVolume->SetTrigger(true);

	GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	SetPhysicsObject(new PhysicsObject(&transform,GetBoundingVolume()));

	physicsObject->SetInverseMass(0.0f);
	physicsObject->InitCubeInertia();
}


