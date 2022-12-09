#pragma once
#include "Player.h"
#include "DestructableObject.h"

using std::vector;

using namespace Maths;
using namespace NCL;
namespace NCL::CSC8503 {
	class Bin : public DestructableObject
	{
	public:
		Bin(const Vector3& position);
		~Bin();

		void Update(float dt);


		void CheckDestroyed();


	protected:
		void AddScoreToPlayer();
		Vector3 initialUp;
		Vector3 currentUp;

		int score = 5;

		Vector3 dimensions;
		float inverseMass = 0.5f;
	};
}