#pragma once
#include "State.h";
#include "StateMachine.h"

namespace NCL {
	namespace CSC8503 {
		class HierarchalStateMachine : public State {
		public:
			HierarchalStateMachine();
			~HierarchalStateMachine();

			void Update(float dt) {
				stateMachine->Update(dt);
			}

			StateMachine* GetStateMachine() {
				return stateMachine;
			}
		protected:
			StateMachine* stateMachine;

		};
	}
}
