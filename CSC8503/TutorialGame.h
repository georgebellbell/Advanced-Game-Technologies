#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include "Human.h"
#include "StateGameObject.h"
#include "Player.h"
#include "Bin.h"
#include "Crate.h"
#include "Powerup.h"
#include "Goose.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "TriggerObject.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateCamera();
			void UpdateKeys();

			void ToggleGravity();

			void InitWorld();
			void InitWorld2();

			void InitFloors();

			void AddBridgeToWorld();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddMainFloorToWorld(const Vector3& position);
			GameObject* AddMazeFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, bool hollow = false);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			Player* AddPlayerToWorld(const Vector3& position, int id);

			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			Bin* AddBinToWorld(const Vector3& position);
			Crate* AddCrateToWorld(const Vector3& position, int nodeReference);
			Human* AddHumanToWorld(const Vector3& position);
			Powerup* AddRandomPowerupToWorld(const Vector3& position);
			Powerup* AddPowerupToWorld(const Vector3& position, PowerupType powerupType);
			Goose* AddGooseToWorld(const Vector3& position);
			TriggerObject* AddTriggerToWorld(const Vector3& position, Vector3 dimensions);
			

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			MeshGeometry*	capsuleMesh = nullptr;
			MeshGeometry*	cubeMesh	= nullptr;
			MeshGeometry*	sphereMesh	= nullptr;

			TextureBase*	basicTex	= nullptr;
			ShaderBase*		basicShader = nullptr;

			//Coursework Meshes
			MeshGeometry*	charMesh	= nullptr;
			MeshGeometry*	enemyMesh	= nullptr;
			MeshGeometry*	bonusMesh	= nullptr;
			MeshGeometry* cylinderMesh  = nullptr;
			MeshGeometry* gooshMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 5, 17);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			vector<DestructableObject*> destructableObjects;
			vector<Powerup*> powerupObjects;
			vector<Human*> humanObjects;
			GameObject* objClosest = nullptr;

			NavigationGrid* worldGrid;
			NavigationGrid* mazeGrid;

			Player* player;

		};
	}
}

