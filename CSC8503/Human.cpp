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


	GeneratePassiveHSM();
	GenerateScaredHSM();
	stateMachine->AddState(passiveHSM);
	stateMachine->AddState(scaredHSM);

	
	stateMachine->AddTransition(new StateTransition(passiveHSM, scaredHSM,
		[&]()->bool
		{
			if (hitByPlayer) {
				movementSpeed = movementSpeed * 2;
				FindPositionAwayFromPlayer();
				renderObject->SetColour(Vector4(1, 0, 0, 1));
				lookingForPlayer = true;
				return true;
			}
			return false;
			// player hits human
			// human target set to point at least a certain distance from player
			// human speed doubled
		}
	));

	stateMachine->AddTransition(new StateTransition(scaredHSM, passiveHSM,
		[&]()->bool
		{
			if (scaredWaitTime <= 0.0f) {
				movementSpeed = movementSpeed / 2;
				FindRandomPosition();
				renderObject->SetColour(Vector4(0, 0, 1, 1));
				hitByPlayer = false;
				lookingForPlayer = false;
				return true;
			}
			return false;
			// hiding time is less than or equal to 0
			// speed back to normal
			//reset collsion detection
		}
	));

	targetPosition = new Vector3();
	targetNodePosition = new Vector3();
	
	*targetPosition = transform.GetPosition();

}

void Human::GeneratePassiveHSM() {
	passiveHSM = new HierarchalStateMachine();
	State* patrol_follow = new State([&](float dt)->void
		{
			if (*targetPosition == transform.GetPosition()) {
				FindRandomPosition();
			}
			MoveTowardsTargetPosition();

		}
	);
	State* patrol_wait = new State([&](float dt)->void
		{
			passiveWaitTime -= dt;
			std::cout << passiveWaitTime << "\n";
		}
	);
	passiveHSM->GetStateMachine()->AddState(patrol_follow);
	passiveHSM->GetStateMachine()->AddState(patrol_wait);

	passiveHSM->GetStateMachine()->AddTransition(new StateTransition(patrol_follow, patrol_wait,
		[&]()->bool
		{
			float distance = (*targetPosition - transform.GetPosition()).Length();
			std::cout << distance << "\n";

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
			if (distance < 10.0f || playerSpotted)
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
	stateMachine->Update(dt);
}

void Human::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName().compare(string("Player")) == 0) {
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

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1), 100.0f);
	}

	
}

void Human::FindPositionAwayFromPlayer()
{
	FindRandomPosition();
	float distance = (*targetPosition - transform.GetPosition()).Length();
	if (distance <= 50.0f) {
		return FindPositionAwayFromPlayer();
	}
}

void Human::MoveTowardsTargetPosition()
{
	*targetNodePosition = path[currentNodeIndex];
		
	
	float distance = (*targetNodePosition - transform.GetPosition()).Length();
	if (distance < 7.0f) {
		currentNodeIndex++;
		if (currentNodeIndex >= path.size()) return;
		*targetNodePosition = path[currentNodeIndex];
	}

	this->ApplyForce(GetForceDirection(*targetNodePosition));

}

