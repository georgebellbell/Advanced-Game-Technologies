#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "PhysicsObject.h"

#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame()	{
	thisServer = nullptr;
	thisClient = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;
}

NetworkedGame::~NetworkedGame()	{
	delete thisServer;
	delete thisClient;
}

void NetworkedGame::StartAsServer() {
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);

	thisServer->RegisterPacketHandler(Received_State, this);
	thisServer->RegisterPacketHandler(Player_Added, this);

	StartLevel();
}

 void NetworkedGame::StartAsClient(char a, char b, char c, char d) {
	 //if (thisServer == nullptr) return;

	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);
	thisClient->RegisterPacketHandler(Player_ID, this);

	player = AddPlayerToWorld(Vector3(100, 0, 100), -1);
	
	LockCameraToObject(player);
	string title("CLIENT");
	//Window::SetTitle(title);

	StartLevel();
}

void NetworkedGame::UpdateGame(float dt) {
	timeToNextPacket -= dt;
	if (timeToNextPacket < 0) {
		if (thisServer) {
			UpdateAsServer(dt);
		}
		else if (thisClient) {
			UpdateAsClient(dt);
		}
		timeToNextPacket += 1.0f / 20.0f; //20hz server/client update
	}

	if (!thisServer && Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		StartAsServer();
	}
	if (!thisClient && Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		StartAsClient(127,0,0,1);
	}

	TutorialGame::UpdateGame(dt);
}

void NetworkedGame::UpdateAsServer(float dt) {
	//player->PlayerMovement(dt);
	world->GetMainCamera()->UpdateCamera(dt);
	thisServer->UpdateServer();
	packetsToSnapshot--;
	if (packetsToSnapshot < 0) {
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else {
		BroadcastSnapshot(true);
	}

	//Debug::Print("Player score:" + std::to_string(player->PlayerScore()), Vector2(5, 5), Debug::RED);

}

void NetworkedGame::UpdateAsClient(float dt) {
	
	if (player->GetWorldID() == -1 && thisClient->connected) {
		AddPlayerToServerPacket addPacket;
		thisClient->UpdateClient();
		addPacket.lastID = 0;
		thisClient->SendPacket(addPacket);
		std::cout << "Requesting player from server\n";
		player->SetWorldID(-2);
		return;
	}
	
	ClientPacket newPacket;
	thisClient->UpdateClient();
	newPacket.lastID = player->GetWorldID(); //You'll need to work this out somehow...


	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
		//fire button pressed!
		newPacket.buttonstates[0] = 1;
	}
	
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		newPacket.buttonstates[1] = 1;
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		newPacket.buttonstates[2] = 1;
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		newPacket.buttonstates[3] = 1;
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		newPacket.buttonstates[4] = 1;
	}

	//float pitch = (Window::GetMouse()->GetRelativePosition().y);
	float yaw = (Window::GetMouse()->GetRelativePosition().x);
	newPacket.yaw = yaw;
	thisClient->SendPacket(newPacket);

	

	Debug::Print("Player score:" + std::to_string(player->PlayerScore()), Vector2(5, 5), Debug::RED);

}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		//TODO - you'll need some way of determining
		//when a player has sent the server an acknowledgement
		//and store the lastID somewhere. A map between player
		//and an int could work, or it could be part of a 
		//NetworkPlayer struct. 
		int playerState = 0;
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			thisServer->SendGlobalPacket(*newPacket);
			delete newPacket;
		}
	}
}

void NetworkedGame::UpdateMinimumState() {
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : stateIDs) {
		minID = min(minID, i.second);
		maxID = max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
	}
}

void NetworkedGame::SpawnPlayer() {
	int playerIndex = serverPlayers.size();
	
	Player* newPlayer = AddPlayerToWorld(Vector3(100, 10, 70 - 10 * playerIndex), playerIndex);

	serverPlayers.insert(std::pair<int, GameObject*>(playerIndex, newPlayer));


	AddPlayerWithID newPacket;
	thisServer->UpdateServer();
	newPacket.lastID = 0;
	newPacket.newPlayerID = playerIndex;
	newPacket.numberOfPlayers = playerIndex;
	thisServer->SendGlobalPacket(newPacket);

	std::cout << "Current player count: " << serverPlayers.size();


}

void NetworkedGame::StartLevel() {
	

}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	if (type == Player_Added) {
		SpawnPlayer();
		return;
	}

	if (type == Player_ID) {
		if (recieved) return;
		recieved = true;
		AddPlayerWithID* recieved_server = (AddPlayerWithID*)payload;
		if (player->GetWorldID() == -2) {
			std::cout << "my player has now been assigned an ID!\n";
			player->SetWorldID(recieved_server->newPlayerID);
			player->GetNetworkObject()->SetNetworkID(recieved_server->newPlayerID);
			serverPlayers.insert(std::pair<int, GameObject*>(recieved_server->newPlayerID, player));

			for (int i = 0; i < recieved_server->numberOfPlayers; i++)
			{
				Player* existingPlayer = AddPlayerToWorld(Vector3(100, 0, 100), i);
				serverPlayers.insert(std::pair<int, GameObject*>(i, existingPlayer));
			}

			recieved = false;
		}
		else{
			std::cout << "Server has created a new player, I now know they exist!\n";
			Player* newPlayer = AddPlayerToWorld(Vector3(100, 0, 100), recieved_server->newPlayerID);
			serverPlayers.insert(std::pair<int, GameObject*>(recieved_server->newPlayerID, newPlayer));
			recieved = false;

		}
		return;
	}

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) continue;
		
		if (((FullPacket*)payload)->objectID == o->GetNetworkID())
		{
			o->ReadPacket(*payload);
		}

		if (type == Received_State) {
			ClientPacket* recieved_client = (ClientPacket*)payload;

			if (dynamic_cast<Player*>((*i))) {
				if (((ClientPacket*)payload)->lastID == o->GetNetworkID()) {
					
					Player* nPlayer = (Player*)(*i);
					for (int i = 0; i < sizeof(recieved_client->buttonstates); i++)
					{
						if (recieved_client->buttonstates[i] == 1) {
							nPlayer->MovePlayer(i);
						}
					}
					Vector3 rotation(0.0f, -recieved_client->yaw * nPlayer->RotationSpeed(), 0.0f);
					//std::cout << -recieved_client->yaw << "\n";

					nPlayer->GetPhysicsObject()->SetAngularVelocity(rotation);
				}
				
			}
		}


		
		
		
	}
}
	

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) { //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID  = a->GetPlayerNum();

		thisClient->SendPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisClient->SendPacket(newPacket);
	}
}