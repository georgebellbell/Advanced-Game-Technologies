#include "Human.h"
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
	GetPhysicsObject()->InitCubeInertia();
	physicsObject->SetElasticity(0.5f);

	objectLayerMask = humanLayerMask;
	objectLayer = enemy;

	//CreateStateMachine
	stateMachine = new StateMachine();


	GeneratePassiveHSM();
	GenerateScaredHSM();
	stateMachine->AddState(passiveHSM);
	stateMachine->AddState(scaredHSM);

	
	stateMachine->AddTransition(new StateTransition(passiveHSM, scaredHSM,
		[&]()->bool
		{
			if (hitByPlayer) {
				movementSpeed = movementSpeed * 3;
				FindPositionAwayFromPlayer();
				renderObject->SetColour(Vector4(1, 0, 0, 1));
				return true;
			}
			return false;
			
		}
	));

	stateMachine->AddTransition(new StateTransition(scaredHSM, passiveHSM,
		[&]()->bool
		{
			if (scaredWaitTime <= 0.0f) {
				movementSpeed = movementSpeed / 3;
				FindRandomPosition();
				renderObject->SetColour(Vector4(0, 0, 1, 1));
				hitByPlayer = false;
				player = nullptr;
				return true;
			}
			return false;
			
		}
	));

	targetPosition = new Vector3();
	targetNodePosition = new Vector3();
	
}

void Human::GeneratePassiveHSM() {
	passiveHSM = new HierarchalStateMachine();
	State* patrol_follow = new State([&](float dt)->void
		{
			if (!beganMoving) {
				FindRandomPosition();
				beganMoving = true;
			}
			MoveTowardsTargetPosition();

		}
	);
	State* patrol_wait = new State([&](float dt)->void
		{
			passiveWaitTime -= dt;
		}
	);
	passiveHSM->GetStateMachine()->AddState(patrol_follow);
	passiveHSM->GetStateMachine()->AddState(patrol_wait);

	passiveHSM->GetStateMachine()->AddTransition(new StateTransition(patrol_follow, patrol_wait,
		[&]()->bool
		{
		

			float distance = (*targetPosition - transform.GetPosition()).Length();

			if (distance < 10.0f) {
				passiveWaitTime = 5.0f;
				return true;
			}
			return false;
		}
	));
	passiveHSM->GetStateMachine()->AddTransition(new StateTransition(patrol_wait, patrol_follow,
		[&]()->bool
		{
			if (passiveWaitTime <= 0.0f) {
				FindRandomPosition();
				return true;
			}
			return false;
		}
	));
}
void Human::GenerateScaredHSM() {
	scaredHSM = new HierarchalStateMachine();
	State* scared_move = new State([&](float dt)->void
		{
			MoveTowardsTargetPosition();
		}
	);
	State* scared_look = new State([&](float dt)->void
		{
			physicsObject->SetAngularVelocity(Vector3(0.0f, 10.0f, 0.0f));
			scaredWaitTime -= dt;
		}
	);
	scaredHSM->GetStateMachine()->AddTransition(new StateTransition(scared_move, scared_look,
		[&]()->bool
		{
			float distance = (*targetPosition - transform.GetPosition()).Length();
			return (distance < 10.0f);
		}
	));
	scaredHSM->GetStateMachine()->AddTransition(new StateTransition(scared_look, scared_move,
		[&]()->bool
		{
			float distance = (player->GetTransform().GetPosition() - transform.GetPosition()).Length();
			if (distance < 10.0f || CanSeePlayer())
			{
				scaredWaitTime = 3.0f;
				FindPositionAwayFromPlayer();
				return true;
			}
			// raytrace from human to player, if successful collision, find new position to hide in return true
			// if no collision, they are far enough away and hidden from view return false
			return false;
		}
	));
	scaredHSM->GetStateMachine()->AddState(scared_move);
	scaredHSM->GetStateMachine()->AddState(scared_look);
}

bool Human::CanSeePlayer()
{
	RayCollision humanLookCollision;
	Vector3 rayPos;
	Vector3 rayDir;

	rayPos = GetTransform().GetPosition();

	rayDir = (player->GetTransform().GetPosition() - rayPos).Normalised();


	Ray r = Ray(rayPos, rayDir);
	if (humanWorld->Raycast(r, humanLookCollision, true, this)) {
		if (dynamic_cast<Player*>((GameObject*)humanLookCollision.node)) {
			return true;
		}
		return false;
	}
	return false;
}

Human::~Human()
{
	delete stateMachine;
	delete scaredHSM;
	delete passiveHSM;

	if (targetPosition != nullptr) {
		delete targetPosition;
	}

	if (targetNodePosition != nullptr) {
		delete targetNodePosition;
	}


}

void Human::Update(float dt)
{
	if (isServer) {
		stateMachine->Update(dt);
	}
}

void Human::OnCollisionBegin(GameObject* otherObject)
{
	if (dynamic_cast<Player*>(otherObject)) {
		hitByPlayer = true;
		player = otherObject;
	}
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

void Human::FindRandomPosition()
{
	//delete targetPosition;
	//targetPosition = new Vector3();
	followPath.Clear();
	path.clear();

	int randomNode = rand() % (humanNavGrid->GridHeight() * humanNavGrid->GridWidth());
	GridNode* allNodes = humanNavGrid->AllNodes();
	GridNode& n = allNodes[randomNode];
	*targetPosition = n.position;

	if (!humanNavGrid->FindPath(transform.GetPosition(), *targetPosition, followPath)) {
		return FindRandomPosition();
	}
	currentNodeIndex = 0;
	//delete targetNodePosition;
	Vector3 pos;
	while (followPath.PopWaypoint(pos)) {
		pos.y = pos.y + 5.0f;
		path.push_back(pos);
	}

	for (int i = 1; i < path.size(); i++)
	{
		Vector3 a = path[i - 1];
		Vector3 b = path[i];

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1), 10.0f);
	}

	
}

void Human::FindPositionAwayFromPlayer()
{
	FindRandomPosition();
	float distance = (*targetPosition - transform.GetPosition()).Length();
	if (distance <= 40.0f) {
		return FindPositionAwayFromPlayer();
	}
}

void Human::MoveTowardsTargetPosition()
{
	if (path.empty()) 
		return;
	*targetNodePosition = path[currentNodeIndex];
		
	
	float distance = (*targetNodePosition - transform.GetPosition()).Length();
	if (distance < 7.0f) {
		currentNodeIndex++;
		if (currentNodeIndex >= path.size()) return;
		*targetNodePosition = path[currentNodeIndex];
	}

	this->ApplyForce(GetForceDirection(*targetNodePosition));

}

