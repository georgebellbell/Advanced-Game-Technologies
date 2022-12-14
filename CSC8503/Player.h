#pragma once
#include "StateGameObject.h"
#include <algorithm>
#include "Window.h"

using std::vector;

using namespace NCL;
using namespace Maths;
namespace NCL::CSC8503 {
	class Powerup;
	class Player : public StateGameObject {
	public:
		Player(const Vector3& position);
		void InitialiseStateMachine();
		~Player();

		void Update(float dt);

		void OnCollisionBegin(GameObject* otherObject);

		void PowerupCollected(Powerup* otherObject);

		float MovementSpeed() const {
			return movementSpeed;
		}

		float JumpPower() const {
			return jumpPower;
		}

		float RotationSpeed() const {
			return rotationSpeed;
		}

		bool Jumping() const {
			return false;//jumping;
		}

		void IsJumping(bool status) {
			jumping = status;
		}

		void AddToTotalScore(int objectScore) {
			playerScore += objectScore;
		}

		int PlayerScore() {
			return playerScore;
		}

		void KillPlayer() {
			dead = true;
			ignoreGravity = true;
			playerScore -= 5;
		}

		bool IsPlayerDead() {
			return dead;
		}

		Vector4 defaultColour = Vector4(1.00, 0.20, 1.00, 1.00);

		void PlayerMovement(float dt);

		void MovePlayer(int keyIndex);

		void RotatePlayer(float pitch, float yaw);

		Vector3 CameraOffset() {
			return cameraOffset;
		}


	protected:
		float meshSize = 1.0f;
		float inverseMass = 0.5f;

		float initialMovementSpeed = 15.0f;
		float initialJumpPower = 150.0f;
		float initialRotationSpeed = 0.9f;


		bool jumping = false;

		int playerScore = 0;

		float powerupTimeRemaining = 0;

		bool dead = false;

		Vector3 origin;
		Vector3 cameraOffset = Vector3(0, 5, 17);
	};
}