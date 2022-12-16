#pragma once

namespace NCL {
	namespace CSC8503 {
		typedef std::function<void(float)> PushDownStateUpdateFunction;
		class PushdownState
		{
		public:
			enum PushdownResult {
				Push, Pop, NoChange
			};
			PushdownState() {}
			
			virtual ~PushdownState() {}

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc) = 0;
		
			virtual void OnAwake() {}
			virtual void OnSleep() {}
			
		protected:
			PushDownStateUpdateFunction func;
		};
	}
}