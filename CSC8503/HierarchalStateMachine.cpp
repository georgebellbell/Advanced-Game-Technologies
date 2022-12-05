#include "HierarchalStateMachine.h"

using namespace NCL;
using namespace CSC8503;

HierarchalStateMachine::HierarchalStateMachine()
{
	stateMachine = new StateMachine();


}

HierarchalStateMachine::~HierarchalStateMachine()
{
	delete stateMachine;
}