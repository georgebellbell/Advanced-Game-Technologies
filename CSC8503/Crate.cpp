#include "Bin.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "Transform.h"
#include "Human.h"
#include "Player.h"

#include "Crate.h"
#include "CollisionVolume.h"

Crate::Crate(const Vector3& position)
{
	Vector3 dimensions = Vector3(4, 4, 4);
	AABBVolume* volume = new AABBVolume(dimensions);
	SetBoundingVolume((CollisionVolume*)volume);

	transform
		.SetPosition(position)
		.SetScale(dimensions * 2);


	SetPhysicsObject(new PhysicsObject(&transform, GetBoundingVolume()));

	physicsObject->SetElasticity(0.2);
	physicsObject->SetInverseMass(inverseMass);
	physicsObject->InitCubeInertia();

	objectLayerMask = destructableLayerMask;
	objectLayer = destructables;
}

Crate::~Crate()
{
}

void Crate::OnCollisionBegin(GameObject* otherObject)
{
	if (dynamic_cast<Player*>(otherObject) && !destroyed) {

		destroyed = true;
		((Player*)otherObject)->AddToTotalScore(score);
	}
	
	if (dynamic_cast<Human*>(otherObject)) {
		Human* human = (Human*)otherObject;
		destroyed = true;
		if (human->ScaryPlayer() != nullptr) {
			//playerCollided = human->ScaryPlayer();
			(human->ScaryPlayer())->AddToTotalScore(score * 2.0f);

		}
	}
	
}
