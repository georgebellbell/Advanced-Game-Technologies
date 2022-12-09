#pragma once
#include "GameObject.h"
#include "Player.h"

enum PowerupType {
	Speed,
	Jump,
	Invisibility,
};

using std::vector;

using namespace NCL;
namespace NCL::CSC8503 {
	class Powerup : public GameObject
	{
	public:
		Powerup(const Vector3& position, PowerupType type );
		~Powerup(){}

		void OnCollisionBegin(GameObject* otherObject) {
			std::cout << "Powerup collected\n";
			collected = true;
		}

		PowerupType GetPowerupType() {
			return type;
		}

		float GetPowerupTime() {
			return powerupTime;
		}

		bool Collected() {
			return collected;
		}

		
	protected:
		PowerupType type;
		float powerupTime = 10.0f;
		float inverseMass = 0;
		float radius = 3.0f;
		bool collected = false;


	};
}


