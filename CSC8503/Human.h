#pragma once
#include "StateGameObject.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"

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

		void SetPlayerSpotted(bool spotted) {
			playerSpotted = spotted;
		}

		bool LookingForPlayer() {
			return lookingForPlayer;
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
		bool playerSpotted = false;
		bool lookingForPlayer = false;

		void GeneratePassiveHSM();
		void GenerateScaredHSM();
		
	};
}



