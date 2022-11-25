#pragma once
namespace NCL {
	enum class VolumeType {
		AABB	= 1,
		OBB		= 2,
		Sphere	= 4, 
		Mesh	= 8,
		Capsule = 16,
		Compound= 32,
		Invalid = 256
	};

	class CollisionVolume
	{
	public:
		CollisionVolume() {
			type = VolumeType::Invalid;
		}
		~CollisionVolume() {}

		VolumeType type;
		void SetDiscrete(bool isDiscrete) { discrete = isDiscrete; }
		bool IsDiscrete() { return discrete; }

		void SetTrigger(bool isTrigger) { trigger = isTrigger; }
		bool IsTrigger() { return trigger; }
	protected:
		bool discrete = true;
		bool trigger = false;
	};
}