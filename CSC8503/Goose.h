#pragma once
#include "BehaviourTreeObject.h"
#include "BehaviourAction.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "Player.h"
#include "GameWorld.h"
#include "DestructableObject.h"


using namespace NCL;
using namespace Maths;
namespace NCL::CSC8503 {
	class Goose : public BehaviourTreeObject
	{
	public:
		Goose(const Vector3& position);
		~Goose();

		//void Update(float dt)

		void OnCollisionBegin(GameObject* otherObject);

		void SetNavigationGrid(NavigationGrid* grid) {
			gooseNavGrid = grid;
		}

		void SetWorld(GameWorld* world) {
			gooseWorld = world;
		}

		void AddObjectToProtect(DestructableObject* object) {
			protectedObjects.push_back(object);
		}

		void SetPlayer(Player* player) {
			this->player = player;
		}

		void AwakenGoose() {
			dormant = false;
			std::cout << "he has awoken...";
		}

		float xOffset;
		float zOffset;
	protected:
		BehaviourSequence* attackSequence;
		BehaviourSequence* chaseSequence;
		BehaviourSequence* investigateSequence;
		BehaviourSequence* patrolSequence;

		BehaviourAction* canSeePlayer;

		bool dormant = true;
		bool alerted = false;
		bool hitPlayer = false;

		float searchTime = 0.0f;

		void InitialiseAttackSequence();
		void InitialiseChaseSequence();
		void InitialiseInvestigateSequence();
		void InitialisePatrolSequence();

		bool GeneratePath();
		void FollowPath();

		int patrolIndex = 0;
		int nodeIndex = 0;
		vector<Vector3> patrolPoints = { Vector3(460,-10,215),Vector3(580.7,-10,214),
										 Vector3(580,-10,100), Vector3(460,-10,100) };

		vector<Vector3> path;
		NavigationGrid* gooseNavGrid;
		NavigationPath followPath;

		Vector3* targetPosition = nullptr;
		Vector3* targetNodePosition = nullptr;

		Player* player;

		GameWorld* gooseWorld;

		bool CanSeePlayer();
		
		Vector3 GetForceDirection(Vector3 position);
		void ApplyForce(Vector3 direction);

		void FindPositionNearby();
		bool CheckForDestroyedObjects();

		vector<DestructableObject*> protectedObjects;
	};


}
