#include "OrientationConstraint.h"
#include "GameObject.h"
#include "PhysicsObject.h"
#include "Debug.h"
#include "Vector3.h"

using namespace NCL;
using namespace Maths;
using namespace CSC8503;


OrientationConstraint::OrientationConstraint(GameObject* a, GameObject* b, Vector3 cAxis)
{
	objectA = a;
	objectB = b;
	constrainedAxis = cAxis;
}

OrientationConstraint::~OrientationConstraint()
{

}

void OrientationConstraint::UpdateConstraint(float dt) {
	
	Vector3 objAUp = objectA->GetTransform().GetOrientation() * constrainedAxis;
	Vector3 objBUp = objectB->GetTransform().GetOrientation() * constrainedAxis;

	float offset = Vector3::Dot(objAUp, objBUp);
	Vector3 offsetDir = Vector3::Cross(objAUp, objBUp);

	if (abs(offset) > 0.0f) {

		PhysicsObject* physA = objectA->GetPhysicsObject();
		PhysicsObject* physB = objectB->GetPhysicsObject();

		Vector3 relativeVelocity = physA->GetAngularVelocity() - physB->GetAngularVelocity();

		float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

		if (constraintMass > 0.0f) {
			float velocityDot = Vector3::Dot(relativeVelocity, offsetDir);

			float biasFactor = 0.01f;
			float bias = -(biasFactor / dt) * offset;

			float lambda = -(velocityDot + bias) / constraintMass;

			Vector3 aImpulse = offsetDir * lambda;
			Vector3 bImpulse = -offsetDir * lambda;

			physA->ApplyAngularImpulse(aImpulse);
			physB->ApplyAngularImpulse(bImpulse);
		}


	}

}

	

	//Vector3 relativePos = objectA->GetTransform().GetPosition() - objectB->GetTransform().GetPosition();

	//float offset = distance - rotationDifference;



	//if (abs(offset) > 0.0f) {

	//	PhysicsObject* physA = objectA->GetPhysicsObject();
	//	PhysicsObject* physB = objectB->GetPhysicsObject();

	//	Vector3 relativeTorque = physA->GetTorque() - physB->GetTorque();
	//	Vector3 offsetDir = relativeTorque.Normalised();

	//	Vector3 relativeVelocity = physA->GetAngularVelocity() - physB->GetAngularVelocity();

	//	float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

	//	if (constraintMass > 0.0f) {
	//		float velocityDot = Vector3::Dot(relativeVelocity, offsetDir);

	//		float biasFactor = 0.01f;
	//		float bias = -(biasFactor / dt) * offset;

	//		float lambda = -(velocityDot + bias) / constraintMass;

	//		Vector3 aImpulse = offsetDir * lambda;
	//		Vector3 bImpulse = -offsetDir * lambda;

	//		physA->ApplyAngularImpulse(aImpulse);
	//		physB->ApplyAngularImpulse(bImpulse);
	//	}


	//}

