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

	SetGameAsServer();
	StartLevel();
}

 void NetworkedGame::StartAsClient(char a, char b, char c, char d) {
	thisClient = new GameClient();
	bool connected = thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	if (connected)
		std::cout << "Client has made a connection\n";
	else {
		std::cout << "Client has not made a connection\n";
	}
	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);
	thisClient->RegisterPacketHandler(Player_ID, this);
	thisClient->RegisterPacketHandler(Game_Information, this);
	thisClient->RegisterPacketHandler(Game_End, this);

	player = AddPlayerToWorld(Vector3(100, 0, 100), -1);
	
	LockCameraToObject(player);
	gameRunning = true;

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
	if (!thisClient && !thisServer) {
		Debug::Print("Welcome to Goat Imitator!", Vector2(25, 5), Vector4(1, 0, 0, 1));
		Debug::Print("Press F9 to start as server", Vector2(25, 35), Vector4(1, 0, 0, 1));
		Debug::Print("Press F10 to start as client", Vector2(25, 45), Vector4(1, 0, 0, 1));
	}


	TutorialGame::UpdateGame(dt);
}

void NetworkedGame::UpdateAsServer(float dt) {
	world->GetMainCamera()->UpdateCamera(dt);
	thisServer->UpdateServer();

	if (gameRunning) {
		gameTime -= dt;
		if (gameTime <= 0.0f || objectsRemaining == 0) {
			std::cout << "GAME HAS FINISHED\n";
			GameEnded gameInfo;
			thisServer->UpdateServer();
			
			int bestScore = 0;
			int bestPlayerID = -2;
			for (auto i : serverPlayers)
			{
				Player* currentPlayer = (Player*)i.second;
				int score = currentPlayer->PlayerScore();
				if (score > bestScore) {
					bestScore = score;
					bestPlayerID = i.first;
				}
			}

			gameInfo.winningPlayerID = bestPlayerID;
			thisServer->SendGlobalPacket(gameInfo);
			gameRunning = false;
			
			
		}
		else {
			GameInformation gameInfo;
			thisServer->UpdateServer();
			gameInfo.timeLeft = gameTime;
			gameInfo.numberOfObjectsLeft = objectsRemaining = destructableObjects.size();
			thisServer->SendGlobalPacket(gameInfo);
			
		}
			
		
	}

	packetsToSnapshot--;
	if (packetsToSnapshot < 0) {
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else {
		BroadcastSnapshot(true);
	}
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

	if (player->GetPlayerMoveStatus() && gameRunning) {
		ClientPacket newPacket;
		thisClient->UpdateClient();
		newPacket.lastID = player->GetWorldID(); 

	
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

		float yaw = (Window::GetMouse()->GetRelativePosition().x);
		newPacket.yaw = yaw;
		thisClient->SendPacket(newPacket);
	}



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

	gameRunning = true;
	
	Player* newPlayer = AddPlayerToWorld(Vector3(100, 0, 100), playerIndex);

	serverPlayers.insert(std::pair<int, GameObject*>(playerIndex, newPlayer));
	goose->AddPlayerToGooseList(newPlayer);

	AddPlayerWithID newPacket;
	thisServer->UpdateServer();
	newPacket.lastID = 0;
	newPacket.newPlayerID = playerIndex;
	newPacket.numberOfPlayers = playerIndex;
	thisServer->SendGlobalPacket(newPacket);

	std::cout << "Current player count: \n" << serverPlayers.size();


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

	if (type == Game_Information) {
		GameInformation* recieved_info = (GameInformation*)payload;
		gameTime = recieved_info->timeLeft;
		objectsRemaining = recieved_info->numberOfObjectsLeft;
		return;
	}
	if (type == Game_End) {
		GameEnded* recieved_info = (GameEnded*)payload;
		winningPlayerID = recieved_info->winningPlayerID;
		gameRunning = false;
		playerCanMove = false;
		std::cout << "GAME OVER, player " << winningPlayerID + 1 << " won\n";
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