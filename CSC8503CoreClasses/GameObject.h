#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "../Build/CSC8503CoreClasses/CollisionLayerMatrix.h"

#include <string>

using std::vector;
using std::string;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;
	class CollisionLayerMatrix;
	class GameObject	{
	public:
		GameObject(std::string name = "", 
					Layers objectLayer = defaultLayer, 
					LayerMasks = defaultLayerMask);
		~GameObject();

		virtual void Update(float dt){}

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

		Layers GetLayer() const {
			return objectLayer;
		}

		LayerMasks GetLayerMask() const {
			return objectLayerMask;
		}

	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool		isActive;
		int			worldID;

		Layers objectLayer = defaultLayer;
		LayerMasks objectLayerMask = defaultLayerMask;

		std::string	name;
		
		Vector3 broadphaseAABB;
	};
}

