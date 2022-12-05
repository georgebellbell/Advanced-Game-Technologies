#pragma once
#include "BehaviourNodeWithChildren.h"

class BehaviourParallel : public BehaviourNodeWithChildren {
public:
	BehaviourParallel(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~BehaviourParallel() {}

	BehaviourState Execute(float dt) override {
		int failedStates = 0;
		int succeededStates = 0;

		for (auto& i : childNodes) {
			BehaviourState nodeState = i->Execute(dt);
			switch (nodeState) {
			case Success: succeededStates++;
			case Failure: failedStates++;
			}
		}

		if (succeededStates > 0) {
			return Success;
		}
		else if (failedStates == childNodes.size()) {
			return Failure;
		}
		return Ongoing;
	}

};
