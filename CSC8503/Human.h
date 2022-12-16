#pragma once
#include "StateGameObject.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "GameWorld.h"
#include "Player.h"

using std::vector;

using namespace NCL;
using namespace Maths;
namespace NCL::CSC8503 {
	class Human : public StateGameObject
	{
	public:
		Human(const Vector3& position);
		~Human();

		void Update(float dt);

		void OnCollisionBegin(GameObject* otherObject);

		void SetTarget(GameObject* player) {
			this->player = player;
		}

		int patrolTarget = 0;

		

		void SetNavigationGrid(NavigationGrid* grid) {
			humanNavGrid = grid;
		}


		void SetWorld(GameWorld* world) {
			humanWorld = world;
		}

		Player* ScaryPlayer() {
			return (Player*)player;
		}

		void SetAsServerAI(bool isServer) {
			this->isServer = isServer;
		}
	protected:
		Vector3 GetForceDirection(Vector3 position);
		void ApplyForce(Vector3 direction);

		vector<Vector3> targetPositions = { Vector3(0,0,0),
										   Vector3(0,0,100),
										   Vector3(100,0,100),
										   Vector3(100,0,0) };

		float meshSize = 3.0f;
		float inverseMass = 0.5f;

		GameObject* player;

		Vector3* targetPosition = nullptr;
		Vector3* targetNodePosition = nullptr;
		int currentNodeIndex;

		vector<Vector3> path;
		void FindRandomPosition();
		void FindPositionAwayFromPlayer();

		void MoveTowardsTargetPosition();

		NavigationGrid* humanNavGrid;
		NavigationPath followPath;

		HierarchalStateMachine* passiveHSM;
		HierarchalStateMachine* scaredHSM;


		float passiveWaitTime = 5.0f;
		float scaredWaitTime = 3.0f;

		bool hitByPlayer = false;
		bool isServer = false;
		bool startedMoving = false;

		void GeneratePassiveHSM();
		void GenerateScaredHSM();

		GameWorld* humanWorld;

		bool CanSeePlayer();

		bool beganMoving = false;
		
	};
}



