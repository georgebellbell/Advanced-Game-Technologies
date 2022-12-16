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

	origin = position;


	InitialiseStateMachine();

}

void Player::InitialiseStateMachine()
{
	stateMachine = new StateMachine();

	HierarchalStateMachine* mainStateMachine = new HierarchalStateMachine();

	State* defaultState = new State([&](float dt)->void
		{
			PlayerMovement(dt);
		}
	);
	mainStateMachine->GetStateMachine()->AddState(defaultState);

	State* powerupState = new State([&](float dt)->void
		{
			PlayerMovement(dt);

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
	
	HierarchalStateMachine* deadStateMachine = new HierarchalStateMachine();

	State* deadState = new State([&](float dt)->void
		{
			Vector3 direction = (origin - transform.GetPosition());
			direction.Normalise();
			physicsObject->AddForce(direction * 20.0f);

		}
	);

	deadStateMachine->GetStateMachine()->AddState(deadState);

	stateMachine->AddState(deadStateMachine);

	stateMachine->AddTransition(new StateTransition(mainStateMachine, deadStateMachine,
		[&]()->bool
		{
			if (dead) {
				objectLayer = deadPlayer;
				ignoreGravity = true;
				renderObject->SetColour(Vector4(0, 0, 0, 0.2f));
				return true;
			}
			return false;
		}
	));
	stateMachine->AddTransition(new StateTransition(deadStateMachine, mainStateMachine,
		[&]()->bool
		{
			float distance = (origin - transform.GetPosition()).Length();

			if (distance <= 10.0f) {
				objectLayer = player;
				renderObject->SetColour(defaultColour);
				dead = false;
				ignoreGravity = false;
				return true;
			}
			return false;
		}
	));
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
	string killFloor("killFloor");
	string goose("goose");

	string collisionObjectName = otherObject->GetName();

	if (collisionObjectName.compare(floor) == 0) {
		jumping = false;
	}

	if (collisionObjectName.compare(powerup) == 0) {
		PowerupCollected((Powerup*)otherObject);
	}

	if ((collisionObjectName.compare(killFloor) == 0 ||  
		collisionObjectName.compare(goose) == 0)
		&& !dead) {
		KillPlayer();
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

void Player::PlayerMovement(float dt)
{
	if (!canMove) return;
	float pitch = (Window::GetMouse()->GetRelativePosition().y);
	float yaw = (Window::GetMouse()->GetRelativePosition().x);

	RotatePlayer(pitch, yaw);

	return; // stuff below is now done server side


	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
		MovePlayer(0);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		MovePlayer(1);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		MovePlayer(2);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		MovePlayer(3);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		MovePlayer(4);
	}
}

void Player::MovePlayer(int keyIndex) {
	Vector3 fwdAxis		= transform.GetOrientation() * Vector3(0, 0, -1);
	Vector3 rightAxis	= transform.GetOrientation() * Vector3(1, 0, 0);
	switch (keyIndex) {
	case 0:
		if (!jumping) {
			physicsObject->AddForce(Vector3(0, 10, 0) * jumpPower);
			jumping = true;
		}
		break;
	case 1: //W
		physicsObject->AddForce(fwdAxis * movementSpeed);
		break;
	case 2: //S
		physicsObject->AddForce(-fwdAxis * movementSpeed);
		break;
	case 3: //D
		physicsObject->AddForce(rightAxis * movementSpeed);
		break;
	case 4: //S
		physicsObject->AddForce(-rightAxis * movementSpeed);
		break;
	}



}

void Player::RotatePlayer(float pitch, float yaw)
{
	//Vector3 rotation(0.0f, -yaw * rotationSpeed, 0.0f); // COMMENT OUT THESE LINES

	//physicsObject->SetAngularVelocity(rotation); // COMMENT OUT THESE LINES


	cameraOffset.y = cameraOffset.y + (pitch * rotationSpeed);

}


