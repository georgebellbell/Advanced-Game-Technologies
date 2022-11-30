#pragma once
#include "Constraint.h"
#include "Vector3.h"

namespace NCL {
	namespace CSC8503 {
		using Maths::Vector3;
		class GameObject;

		class OrientationConstraint : public Constraint
		{
		public:
			OrientationConstraint(GameObject* a, GameObject* b, Vector3 cAxis);
			~OrientationConstraint();

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;
			Vector3 constrainedAxis;
		};
	}
}

