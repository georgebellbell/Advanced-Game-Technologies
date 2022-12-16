#pragma once
#include "Player.h"
#include "DestructableObject.h"

using std::vector;

using namespace Maths;
using namespace NCL;
namespace NCL::CSC8503 {
	class Crate : public DestructableObject
	{
	public:
		Crate(const Vector3& position);
		~Crate();

		void OnCollisionBegin(GameObject* otherObject);


		void CheckDestroyed() {}

	protected:
		void AddScoreToPlayer() {}

		int score = 3;
		Vector3 dimensions;
		float inverseMass = 0.5f;
	};
}