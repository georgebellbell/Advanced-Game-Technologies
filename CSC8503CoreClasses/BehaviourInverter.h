#pragma once
#include "BehaviourNodeWithChildren.h"

class BehaviourInverter : public BehaviourNodeWithChildren {
public:
	BehaviourInverter(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~BehaviourInverter() {}

	BehaviourState Execute(float dt) override {
		for (auto& i : childNodes) {
			BehaviourState nodeState = i->Execute(dt);

			if (nodeState == Success) return Failure;
			if (nodeState == Failure) return Success;
		}
		return Ongoing;
	}

};
