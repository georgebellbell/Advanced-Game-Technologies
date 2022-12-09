#include "Player.h"
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
#include "Powerup.h";

Player::Player(const Vector3& position)
{
	name = "Player";
	CapsuleVolume* volume = new CapsuleVolume(1.0f, 0.5f);

	SetBoundingVolume((CollisionVolume*)volume);

	GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);


	SetPhysicsObject(new PhysicsObject(&transform, GetBoundingVolume()));

	physicsObject->SetElasticity(0.2);
	physicsObject->SetInverseMass(inverseMass);
	physicsObject->InitCubeInertia();

	objectLayer = player;
	objectLayerMask = playerLayerMask;

	movementSpeed = initialMovementSpeed;
	jumpPower = initialJumpPower;
	rotationSpeed = initialRotationSpeed;


	InitialiseStateMachine();

}

void Player::InitialiseStateMachine()
{
	stateMachine = new StateMachine();

	HierarchalStateMachine* mainStateMachine = new HierarchalStateMachine();

	State* defaultState = new State([&](float dt)->void
		{
			
		}
	);
	mainStateMachine->GetStateMachine()->AddState(defaultState);

	State* powerupState = new State([&](float dt)->void
		{
			powerupTimeRemaining -= dt;
		}
	);
	mainStateMachine->GetStateMachine()->AddState(powerupState);

	mainStateMachine->GetStateMachine()->AddTransition(new StateTransition(defaultState, powerupState,
		[&]()->bool
		{
			return powerupTimeRemaining > 0.0f;
		}
	));
	mainStateMachine->GetStateMachine()->AddTransition(new StateTransition(powerupState, defaultState,
		[&]()->bool
		{
			if (powerupTimeRemaining <= 0.0f) {
				powerupTimeRemaining = 0.0f;
				movementSpeed = initialMovementSpeed;
				jumpPower = initialJumpPower;
				GetRenderObject()->SetColour(defaultColour);
				objectLayer = player;
				return true;
			}
			return false;
		}
	));

	stateMachine->AddState(mainStateMachine);
	

}



Player::~Player()
{
	delete stateMachine;
}

void Player::Update(float dt)
{
	stateMachine->Update(dt);
}

void Player::OnCollisionBegin(GameObject* otherObject)
{
	string floor("floor");
	string powerup("powerup");

	string collisionObjectName = otherObject->GetName();

	if (collisionObjectName.compare(floor) == 0) {
		jumping = false;
	}

	if (collisionObjectName.compare(powerup) == 0) {
		PowerupCollected((Powerup*)otherObject);
	}
}

void Player::PowerupCollected(Powerup* powerup)
{
	PowerupType type = powerup->GetPowerupType();
	powerupTimeRemaining = powerup->GetPowerupTime();

	renderObject->SetColour(powerup->GetRenderObject()->GetColour());

	switch (type) {
	case Speed:
		movementSpeed = initialMovementSpeed * 3.5f;
		break;
	case Jump:
		jumpPower = initialJumpPower * 10.0f;
		break;
	case Invisibility:
		objectLayer = invisiblePlayer;
		break;
	}
}

