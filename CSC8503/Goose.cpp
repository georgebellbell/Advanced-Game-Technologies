#include "Goose.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "BehaviourParallel.h"
#include "BehaviourInverter.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "Transform.h"
#include "CollisionVolume.h"

using namespace NCL;
using namespace CSC8503;

Goose::Goose(const Vector3& position)
{
	Vector3 sphereSize = Vector3(2.0f, 2.0f, 2.0f);
	SphereVolume* volume = new SphereVolume(2.0f);
	SetBoundingVolume((CollisionVolume*)volume);
	transform
		.SetScale(sphereSize)
		.SetPosition(position);

	SetPhysicsObject(new PhysicsObject(&transform, GetBoundingVolume()));

	physicsObject->SetInverseMass(1.0f);
	physicsObject->InitSphereInertia();

	objectLayerMask = enemyLayerMask;
	objectLayer = enemy;

	targetPosition = new Vector3();
	targetNodePosition = new Vector3();

	canSeePlayer = new BehaviourAction("Player spotted",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				bool playerVisible = CanSeePlayer();
				if (playerVisible) {
					//std::cout << "Player Spotted!\n";
					state = Success;
				}
				else {
					//std::cout << "Player not Spotted!\n";
					state = Failure;
				}
			}
			return state;
		});

	InitialiseAttackSequence();
	InitialiseChaseSequence();
	InitialiseInvestigateSequence();
	InitialisePatrolSequence();
	
	BehaviourSelector* actionSelector = new BehaviourSelector("Action Selection");
	actionSelector->AddChild(attackSequence);
	actionSelector->AddChild(chaseSequence);
	actionSelector->AddChild(investigateSequence);
	actionSelector->AddChild(patrolSequence);

	BehaviourAction* checkIfDormant = new BehaviourAction("Checking if goose is asleep...",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				if (dormant) {
					//std::cout << "Goose is still asleep!\n";
					return Failure;
				}
				std::cout << "Goose is awake, and is on the move!\n";
				return Success;
			}
			return state;
		});

	rootSequence = new BehaviourSequence("Root");
	rootSequence->AddChild(checkIfDormant);

	rootSequence->AddChild(actionSelector);	
}

Goose::~Goose()
{
	delete rootSequence;
	delete chaseSequence;
	delete attackSequence;
	delete investigateSequence;
	delete patrolSequence;
	delete canSeePlayer;

	if (targetPosition != nullptr) {
		delete targetPosition;
	}

	if (targetNodePosition != nullptr) {
		delete targetNodePosition;
	}
}

void Goose::OnCollisionBegin(GameObject* otherObject)
{
	///*if (otherObject->GetName().compare(string("Player")) == 0) {
	//	hitPlayer = true;
	//	player = (Player*)otherObject;
	//}*/
}

void Goose::InitialiseAttackSequence()
{
	attackSequence = new BehaviourSequence("Attack Sequence");

	BehaviourAction* checkPlayerInRange = new BehaviourAction("Checking player is in range",
		[&](float dt, BehaviourState state)->BehaviourState {
			canSeePlayer->Reset();
			if (state == Initialise) {
				std::cout << "Preparing to attack!\n";
				state = Ongoing;
			}
			else if (state == Ongoing) {
				float distanceToPlayer = (player->GetTransform().GetPosition()
					- transform.GetPosition()).Length();
				if (distanceToPlayer <= 5.0) {
					std::cout << "In range to attack!\n";
					return Success;
				}
				std::cout << "Not in range to attack\n";
				return Failure;
			}
			return state;
		});

	BehaviourAction* attackPlayer = new BehaviourAction("Attacking player",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				// take points from player and reset their position
				std::cout << "Attacking Player\n";
				player->KillPlayer();
				//hitPlayer = false;
				state = Success;
			}
			return state;
		});

	attackSequence->AddChild(canSeePlayer);
	attackSequence->AddChild(checkPlayerInRange);
	attackSequence->AddChild(attackPlayer);
}

void Goose::InitialiseChaseSequence()
{
	chaseSequence = new BehaviourSequence("Chase Sequence");

	BehaviourAction* moveTowardsPlayer = new BehaviourAction("Chasing player",
		[&](float dt, BehaviourState state)->BehaviourState {
			canSeePlayer->Reset();
			if (state == Initialise) {
				std::cout << "Chasing Player\n";
				*targetPosition = player->GetTransform().GetPosition();
				if (!GeneratePath()) {
					state = Failure;
					return state;
				}
				state = Ongoing;
			}
			else if (state == Ongoing) {
				FollowPath();
				float distance = (*targetPosition - transform.GetPosition()).Length();
				if (distance < 10.0f)
					state = Success;
			}
			return state;
		});

	chaseSequence->AddChild(canSeePlayer);
	chaseSequence->AddChild(moveTowardsPlayer);

}

void Goose::InitialiseInvestigateSequence()
{
	BehaviourInverter* cannotSeePlayer = new BehaviourInverter("Player not spotted");
	cannotSeePlayer->AddChild(canSeePlayer);

	investigateSequence = new BehaviourSequence("Investigate Sequence");

	BehaviourSequence* approachDisturbanceSequence = new BehaviourSequence("Approaching disturbance");

	BehaviourAction* alertedToEvent = new BehaviourAction("Goose is alerted",
		[&](float dt, BehaviourState state)->BehaviourState {
				canSeePlayer->Reset();

				if (alerted) return state;
				if (CheckForDestroyedObjects()) {
					std::cout << "Goose has been alerted to a sound!\n";
					alerted = true;
					state = Success;
				}
				else
				{
					state = Failure;
				}
				return state;
			});

	BehaviourAction* moveTowardsDisturbance = new BehaviourAction("Investigating Sound",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Investigating the disturbance\n";
				//*targetPosition = Vector3(); // move to sound
				if (!GeneratePath()) {
					state = Failure;
					return state;
				}

				state = Ongoing;
			}
			else if (state == Ongoing) {
				FollowPath();
				float distance = (*targetPosition - transform.GetPosition()).Length();
				if (distance < 10.0f) state = Success;

			}
			return state;
		});

	approachDisturbanceSequence->AddChild(alertedToEvent);
	approachDisturbanceSequence->AddChild(moveTowardsDisturbance);


	BehaviourParallel* lookAroundArea = new BehaviourParallel("Searching around for a bit");


	BehaviourAction* searchTimeCompleted = new BehaviourAction("Wasting time looking around",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Beginning Search\n";

				searchTime = 5.0f;

				state = Ongoing;
			}
			else if (state == Ongoing) {

				searchTime -= dt;
				std::cout << "Search time\n" << searchTime;

				if (searchTime <= 0.0f) {
					state = Success;
				}
				
			}
			return state;
		});

	BehaviourAction* movingTowardsInvestigationSpot = new BehaviourAction("Investigating Sound",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking over here\n";
				FindPositionNearby(); // move to point of interest
				if (!GeneratePath()) {
					state = Failure;
					return state;
				}

				state = Ongoing;
			}
			else if (state == Ongoing) {
				FollowPath();
				float distance = (*targetPosition - transform.GetPosition()).Length();
				if (distance < 10.0f) state = Success;

			}
			return state;
		});


	BehaviourAction* ceaseInvestigation = new BehaviourAction("Stop investigating",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "No longer investigating noise\n";
				alerted = false;
				state = Success;
			}
			return state;
		});

	lookAroundArea->AddChild(searchTimeCompleted);
	lookAroundArea->AddChild(movingTowardsInvestigationSpot);

	investigateSequence->AddChild(cannotSeePlayer);
	investigateSequence->AddChild(approachDisturbanceSequence);
	investigateSequence->AddChild(lookAroundArea);
	investigateSequence->AddChild(ceaseInvestigation);




}

void Goose::InitialisePatrolSequence()
{
	BehaviourInverter* cannotSeePlayer = new BehaviourInverter("Player not spotted");
	cannotSeePlayer->AddChild(canSeePlayer);

	patrolSequence = new BehaviourSequence("Patrol Sequence");

	BehaviourAction* moveToPatrolPoint = new BehaviourAction("Patrolling",
		[&](float dt, BehaviourState state)->BehaviourState {
			canSeePlayer->Reset();
			if (state == Initialise) {
				std::cout << "Moving towards patrol point\n";

				*targetPosition = patrolPoints[patrolIndex];
				if (!GeneratePath()) {
					state = Failure;
					return state;
				}
				state = Ongoing;
			}
			else if (state == Ongoing) {
				FollowPath();
				float distance = (*targetPosition - transform.GetPosition()).Length();
				if (distance < 10.0f) {
					patrolIndex = (patrolIndex + 1) % patrolPoints.size();
					state = Success;

				}
				
			}
			return state;
		});

	patrolSequence->AddChild(cannotSeePlayer);
	patrolSequence->AddChild(moveToPatrolPoint);
}

bool Goose::GeneratePath()
{
	nodeIndex = 0;
	followPath.Clear();
	path.clear();

	if (!gooseNavGrid->FindOffsetPath(transform.GetPosition(), *targetPosition, followPath, xOffset, zOffset)) {
		std::cout << *targetPosition << "\n";
		std::cout << transform.GetPosition() << "\n";

		return false;
	}

	Vector3 pos;
	while (followPath.PopWaypoint(pos)) {
		pos.y = pos.y + 5.0f;
		path.push_back(pos);
	}

	for (int i = 1; i < path.size(); i++)
	{
		Vector3 a = path[i - 1];
		Vector3 b = path[i];

		Debug::DrawLine(a, b, Vector4(1, 0, 0, 1), 30.0f);
	}

	return true;
}

void Goose::FollowPath()
{
	*targetNodePosition = path[nodeIndex];


	float distance = (*targetNodePosition - transform.GetPosition()).Length();
	if (distance < 8.0f) {
		if (nodeIndex + 1 >= path.size()) return;
		nodeIndex++;
		*targetNodePosition = path[nodeIndex];
	}

	this->ApplyForce(GetForceDirection(*targetNodePosition));
}

Vector3 Goose::GetForceDirection(Vector3 position)
{
	Vector3 direction = position - transform.GetPosition();
	return direction.Normalised();
}

void Goose::ApplyForce(Vector3 direction)
{
	GetPhysicsObject()->AddForce(direction * 10.0f);
}

void Goose::FindPositionNearby()
{
	int randomNode = rand() % (gooseNavGrid->GridHeight() * gooseNavGrid->GridWidth());
	GridNode* allNodes = gooseNavGrid->AllNodes();
	GridNode& n = allNodes[randomNode];
	*targetPosition = n.position;
	if (!GeneratePath()) return FindPositionNearby();
	
	float distance = (*targetPosition - transform.GetPosition()).Length();
	if (distance >= 30.0f && distance <= 10.0f) {
		return FindPositionNearby();
	}


}

bool Goose::CheckForDestroyedObjects()
{
	for (auto i : protectedObjects) {
		if (i->Destroyed()) {
			*targetPosition = i->GetTransform().GetPosition();
			protectedObjects.erase(std::remove(protectedObjects.begin(),
				protectedObjects.end(), i), protectedObjects.end());
			return true;
		}
	}
	return false;
	
}

bool Goose::CanSeePlayer()
{
	RayCollision gooseLookCollision;
	Vector3 rayPos;
	Vector3 rayDir;

	rayPos = GetTransform().GetPosition();

	rayDir = (player->GetTransform().GetPosition() - rayPos).Normalised();

	//Debug::DrawLine(player->GetTransform().GetPosition(), rayPos, Vector4(0, 0, 1, 1), 15.0f);
	Ray r = Ray(rayPos, rayDir);
	if (gooseWorld->Raycast(r, gooseLookCollision, true, this)) {

		GameObject* gameOb = (GameObject*)gooseLookCollision.node;
		if (dynamic_cast<Player*>((GameObject*)gooseLookCollision.node)) {
			return true;
		}
		return false;
	}
	return false;
}
