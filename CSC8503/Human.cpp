#include "Human.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "Transform.h"
#include "CollisionVolume.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "HierarchalStateMachine.h"


using namespace NCL;
using namespace CSC8503;

Human::Human(const Vector3& position)
{
	//CreateObject
	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);
	SetPhysicsObject(new PhysicsObject(&transform, this->boundingVolume));
	GetPhysicsObject()->SetInverseMass(inverseMass);
	GetPhysicsObject()->InitSphereInertia();

	//CreateStateMachine
	stateMachine = new StateMachine();

	HierarchalStateMachine* patrolState = new HierarchalStateMachine();
	State* patrol_follow = new State([&](float dt)->void
		{
			Vector3 target = targetPositions[patrolTarget];
			this->ApplyForce(GetForceDirection(target));
			float distance = (targetPositions[patrolTarget] - transform.GetPosition()).Length();
			if (distance < 10.0f) {
				patrolTarget = (patrolTarget + 1) % targetPositions.size();
			}
		}
	);
	patrolState->GetStateMachine()->AddState(patrol_follow);


	HierarchalStateMachine* chaseState = new HierarchalStateMachine();
	State* chase_follow = new State([&](float dt)->void
		{
			Vector3 targetPos = target->GetTransform().GetPosition();
			this->ApplyForce(GetForceDirection(targetPos));
		}
	);
	chaseState->GetStateMachine()->AddState(chase_follow);


	stateMachine->AddState(patrolState);
	stateMachine->AddState(chaseState);

	stateMachine->AddTransition(new StateTransition(patrolState, chaseState,
		[&]()->bool
		{
			float distance = (target->GetTransform().GetPosition() - transform.GetPosition()).Length();
			return distance < 10.0f;
		}
	));

	stateMachine->AddTransition(new StateTransition(chaseState, patrolState,
		[&]()->bool
		{
			float distance = (target->GetTransform().GetPosition() - transform.GetPosition()).Length();
			return distance >= 20.0f;
		}
	));
	
	

}

Human::~Human()
{
	delete stateMachine;
}

void Human::Update(float dt)
{
	stateMachine->Update(dt);
}

Vector3 Human::GetForceDirection(Vector3 position)
{
	Vector3 direction = position - transform.GetPosition();
	return direction.Normalised();
}

void Human::ApplyForce(Vector3 direction)
{
	GetPhysicsObject()->AddForce(direction * 10.0f);
}
