#pragma once
#include "GameObject.h"
#include "Goose.h"

using namespace NCL;
namespace NCL::CSC8503 {
	//typedef std::function<void(GameObject*)> TriggerCollisionFunction;
	class TriggerObject : public GameObject
	{
	public:
		TriggerObject(const Vector3& position, Vector3 dimensions);
		~TriggerObject() {}

		void OnCollisionBegin(GameObject* otherObject) {
			if (dynamic_cast<Player*>((GameObject*)otherObject)) {
				Goose* goose = (Goose*)objectToEffect;
				goose->AwakenGoose();
			}
		}


		void SetObjectToEffect(GameObject* obj) {
			objectToEffect = obj;
		}

		GameObject* ObjectToEffect() {
			return objectToEffect;
		}
	protected:
		string objectToInteractWith = "Player";
		GameObject* objectToEffect;

		bool triggered = false;

	};
}
