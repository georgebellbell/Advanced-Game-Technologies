#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "BehaviourParallel.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

vector<Vector3> testNodes;
void TestPathfinding() {
	NavigationGrid grid("TestGrid1.txt");

	NavigationPath outPath;

	Vector3 startPos(80, 0, 10);
	Vector3 endPos(80, 0, 80);

	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}

void DisplayPathfinding() {
	for (int i = 1; i < testNodes.size(); i++)
	{
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}

	testNodes.clear();
}

/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/

void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();
	int data = 0;

	State* A = new State([&](float dt)->void {
		std::cout << "I'm in state A!\n";
		data++;
		}
	);
	State* B = new State([&](float dt)->void {
		std::cout << "I'm in state B!\n";
		data--;
		}
	);

	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool {
		return data > 10;
		}
	);
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool {
		return data < 0;
		}
	);

	testMachine->AddState(A);
	testMachine->AddState(B);
	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);;

	for (int i = 0; i < 100; i++)
	{
		testMachine->Update(1.0f);
	}
}	


void TestBehaviourTree() {
	float behaviourTimer;
	float distanceToTarget;
	BehaviourAction* findKey = new BehaviourAction("Find Key",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for a key\n";
				behaviourTimer = rand() % 100;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				behaviourTimer -= dt;
				if (behaviourTimer <= 0.0f) {
					std::cout << "Found a key!\n";
					return Success;
				}
			}
			return state;
		});

	BehaviourAction* goToRoom = new BehaviourAction("Go To Room",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Going to the loot room!\n";
				state = Ongoing;
			}
			else if (state == Ongoing) {
				behaviourTimer -= dt;
				if (behaviourTimer <= 0.0f) {
					std::cout << "Reached room!\n";
					return Success;
				}
			}
			return state;
		});

	BehaviourAction* openDoor = new BehaviourAction("OpenDoor",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Opening Door!\n";
				state = Success;
			}
			return state;
		});

	BehaviourAction* lookForTreasure = new BehaviourAction("Look for Treasure",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for treasure!\n";
				state = Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some treasure!\n";
					return Success;
				}
				std::cout << "No Treasure in here...\n";
				return Failure;
			}
			return state;
		});

	BehaviourAction* lookForItems = new BehaviourAction("Look for Items",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for items!\n";
				state = Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some items!\n";
					return Success;
				}
				std::cout << "No items in here...\n";
				return Failure;
			}
			return state;
		});

	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	BehaviourParallel* parallel = new BehaviourParallel("Loot Selection");
	parallel->AddChild(lookForTreasure);
	parallel->AddChild(lookForItems);

	BehaviourSequence* rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(parallel);

	for (int i = 0; i < 5; i++)
	{
		rootSequence->Reset();
		behaviourTimer = 0.0f;
		distanceToTarget = rand() % 250;
		BehaviourState state = Ongoing;
		std::cout << "We are going on an adventure!\n";
		while (state == Ongoing)
		{
			state = rootSequence->Execute(1.0f);
		}
		if (state == Success) {
			std::cout << "Adventure was successful\n";
		}
		else if (state == Failure) {
			std::cout << "What a waste of time this adventure was!\n";
		}
	}
	std::cout << "All done!\n";
}



class TestPacketReceiver : public PacketReceiver {
public:
	TestPacketReceiver(string name) {
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;

			string msg = realPacket->GetStringFromData();

			std::cout << name << " received message: " << msg << std::endl;
		}
	}
protected:
	string name;
};

void TestNetworking() {
	NetworkBase::Initialise();

	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver1("Client 1");
	TestPacketReceiver clientReceiver2("Client 2");

	int port = NetworkBase::GetDefaultPort();

	GameServer* server = new GameServer(port, 2);
	GameClient* client1 = new GameClient();
	GameClient* client2 = new GameClient();

	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client1->RegisterPacketHandler(String_Message, &clientReceiver1);
	client2->RegisterPacketHandler(String_Message, &clientReceiver2);


	bool canConnect = client1->Connect(127, 0, 0, 1, port);
	canConnect = client2->Connect(127, 0, 0, 1, port);

	for (int i = 0; i < 100; i++)
	{
		StringPacket p = StringPacket("Server says hello! " + std::to_string(i));
		server->SendGlobalPacket(p);

		p = StringPacket("Client 1 says hello! " + std::to_string(i));
		client1->SendPacket(p);

		server->UpdateServer();
		client1->UpdateClient();

		p = StringPacket("Client 2 says hello! " + std::to_string(i));
		client2->SendPacket(p);

		server->UpdateServer();
		client2->UpdateClient();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	NetworkBase::Destroy();
}


int main() {
	
	//TestNetworking();
	//return 0;
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
	if (!w->HasInitialised()) {
		return -1;
	}	
	//TestPushdownAutomata(w);

	w->ShowOSPointer(true);
	w->LockMouseToWindow(false);

	//TutorialGame* g = new TutorialGame();

	NetworkedGame* g = new NetworkedGame();
	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		//TestPathfinding();
		//DisplayPathfinding();
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		/*if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
			w->SetWindowPosition(0, 0);
		}*/

		//w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
		w->SetTitle(g->GetGameType());

		
		

		g->UpdateGame(dt);
	}
	Window::DestroyGameWindow();
}