#pragma once
#include "StateGameObject.h"

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

		void SetTarget(GameObject* target) {
			this->target = target;
		}

		int patrolTarget = 0;

	protected:
		Vector3 GetForceDirection(Vector3 position);
		void ApplyForce(Vector3 direction);

		vector<Vector3> targetPositions = {Vector3(0,0,0),
										   Vector3(0,0,100),
										   Vector3(100,0,100), 
										   Vector3(100,0,0) };

		float meshSize = 3.0f;
		float inverseMass = 0.5f;

		GameObject* target;
	};
}



