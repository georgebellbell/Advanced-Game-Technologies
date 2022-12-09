#pragma once
#include "GameObject.h"
#include "Player.h"

using std::vector;

using namespace Maths;
using namespace NCL;
namespace NCL::CSC8503 {
	class DestructableObject : public GameObject
	{
	public:
		DestructableObject() {}
		~DestructableObject() {};

		virtual void OnCollisionBegin(GameObject* otherObject) {
			if (otherObject->GetName().compare(string("Player")) == 0) {
				playerCollided = (Player*)otherObject;
			}
		}
		virtual void CheckDestroyed() = 0;

		bool Destroyed() {
			return destroyed;
		}
		int nodeReference;
	protected:
		virtual void AddScoreToPlayer() = 0;

		Player* playerCollided; 
		int score = 1;
		bool destroyed = false;
	};
}

