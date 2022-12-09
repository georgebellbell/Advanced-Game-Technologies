#include "Bin.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "Transform.h"
#include "CollisionVolume.h"

Bin::Bin(const Vector3& position)
{
	Vector3 dimensions = Vector3(1, 2, 1);
	AABBVolume* volume = new AABBVolume(dimensions);
	SetBoundingVolume((CollisionVolume*)volume);

	transform
		.SetPosition(position)
		.SetScale(dimensions * 2);

	
	SetPhysicsObject(new PhysicsObject(&transform, GetBoundingVolume()));

	physicsObject->SetElasticity(0.2);
	physicsObject->SetInverseMass(inverseMass);
	physicsObject->InitCubeInertia();

	initialUp = transform.GetOrientation() * Vector3(0, 1, 0);
}

Bin::~Bin()
{
}

void Bin::Update(float dt)
{
	//Call check update
	if (!physicsObject->IsSleeping()) {
		CheckDestroyed();
	}
}

void Bin::CheckDestroyed()
{
	currentUp = transform.GetOrientation() * Vector3(0, 1, 0);
	float difference = Vector3::Dot(currentUp, initialUp);

	if (difference < 0.5 && !destroyed) {
		destroyed = true;
		AddScoreToPlayer();
	}
	
}

void Bin::AddScoreToPlayer()
{
	if (playerCollided == nullptr) { //object destroyed without being hit by a player. must have been the wind...
		return;
	}
	playerCollided->AddToTotalScore(score);
	
}
