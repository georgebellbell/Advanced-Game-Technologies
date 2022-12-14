#pragma once
#include "GameObject.h"
#include "BehaviourSequence.h"
namespace NCL::CSC8503 {
	class BehaviourTreeObject : public GameObject {
	public:
		BehaviourTreeObject() {}
		~BehaviourTreeObject() {}

		virtual void Update(float dt) { 

			BehaviourState state = rootSequence->Execute(dt);
			if (state == Success || state == Failure) rootSequence->Reset();
		}

	protected:
		BehaviourSequence* rootSequence;
	};
}
