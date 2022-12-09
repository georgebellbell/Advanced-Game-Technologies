#include "Bin.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "Transform.h"

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
}

Crate::~Crate()
{
}

void Crate::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName().compare(string("Player")) == 0 && !destroyed) {
		playerCollided = (Player*)otherObject;
		destroyed = true;
		AddScoreToPlayer();
	}
}



void Crate::AddScoreToPlayer()
{
	if (playerCollided == nullptr) { //object destroyed without being hit by a player. must have been the wind...
		return;
	}
	playerCollided->AddToTotalScore(score);
}
