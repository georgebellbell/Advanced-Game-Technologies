#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h" 
#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "Player.h"
#include "NetworkObject.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"


using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= true;
	inSelectionMode = false;

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");
	cylinderMesh = renderer->LoadMesh("cylinder.msh");
	gooshMesh = renderer->LoadMesh("goose.msh");


	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	//InitWorld();
	InitWorld2();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;
	delete cylinderMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	/*if (inSelectionMode) {
		
	}*/
	if (lockedObject != nullptr && !inSelectionMode) {
		UpdateCamera();
	}

	UpdateKeys();

	if (testStateObject) {
		testStateObject->Update(dt);
	}

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	//SelectObject();
	//MoveSelectedObject();

	//Check destructables
	for (DestructableObject* i : destructableObjects)
	{
		if (i->Destroyed()) {
			GridNode* allNodes = worldGrid->AllNodes(); 
			allNodes[i->nodeReference].type = '.';
			worldGrid->BuildNodeConnections(); // rebuild node connections

			destructableObjects.erase(std::remove(destructableObjects.begin(), 
				destructableObjects.end(), i), destructableObjects.end());
			world->RemoveGameObject(i);
		}
	}

	// check powerups
	for (Powerup* i : powerupObjects)
	{
		if (i->Collected()) {
			powerupObjects.erase(std::remove(powerupObjects.begin(),
				powerupObjects.end(), i), powerupObjects.end());
			world->RemoveGameObject(i);
		}
	}

	if (player != nullptr) 
		Debug::Print("Player score:" + std::to_string(player->PlayerScore()), Vector2(5, 5), Debug::RED);


	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld2(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		ToggleGravity();

	}
	{
		//Running certain physics updates in a consistent order might cause some
		//bias in the calculations - the same objects might keep 'winning' the constraint
		//allowing the other one to stretch too much etc. Shuffling the order so that it
		//is random every frame can help reduce such bias.
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
			world->ShuffleConstraints(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
			world->ShuffleConstraints(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
			world->ShuffleObjects(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
			world->ShuffleObjects(false);
		}
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::ToggleGravity()
{
	useGravity = !useGravity; //Toggle gravity!
	physics->UseGravity(useGravity);

	std::vector<GameObject*>::const_iterator first, last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; i++)
	{
		(*i)->GetPhysicsObject()->SetToSleep(false);
	}
}

void TutorialGame::LockedObjectMovement() {

	Player* player = (Player*)lockedObject;
	if (player->IsPlayerDead()) return;

	
	
	
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::UpdateCamera()
{
	if (player == nullptr) return;
	Vector3 camLockOffset = player->CameraOffset();
	Vector3 objPos = lockedObject->GetTransform().GetPosition();
	Vector3 camPos = objPos + lockedObject->GetTransform().GetOrientation() * camLockOffset;
	float defaultDistance = (camPos - objPos).Length();
	RayCollision gooseLookCollision;
	Vector3 rayPos;
	Vector3 rayDir;
	rayPos = objPos;
	rayDir = (camPos - rayPos).Normalised();
	Ray r = Ray(rayPos, rayDir);
	if (world->Raycast(r, gooseLookCollision, true, lockedObject)) {
		float distance = gooseLookCollision.rayDistance;
		if (distance < defaultDistance) camPos = gooseLookCollision.collidedAt;
	}
	world->GetMainCamera()->SetPosition(camPos);

	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));
	Matrix4 modelMat = temp.Inverse();
	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); //nearly there now!

	world->GetMainCamera()->SetPitch(angles.x);
	world->GetMainCamera()->SetYaw(angles.y);
}


void TutorialGame::InitWorld2() {
	world->ClearAndErase();
	physics->Clear();
	destructableObjects.clear();
	powerupObjects.clear();
	humanObjects.clear();

	//player = AddPlayerToWorld(Vector3(100, 0, 100), -1);
	//LockCameraToObject(player);

	Human* human = AddHumanToWorld(Vector3(100, 0, 80));


	
	worldGrid = new NavigationGrid("TestGrid2.txt");
	GridNode* allNodes = worldGrid->AllNodes();
	int gridWidth = worldGrid->GridWidth();
	int gridHeight = worldGrid->GridHeight();
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];
			Vector3 nodePosition = n.position;
			GameObject* object;
			switch (n.type) {
			case '.':
				//clear area!
				break;
			case 'x':
				//wall
				object = AddCubeToWorld(Vector3(nodePosition.x, nodePosition.y + 10, nodePosition.z),
							   Vector3(5, 10, 5), 0.0f);
				object->GetRenderObject()->SetColour(Vector4(0.57, 0.64, 0.69, 1.0f));
				break;
			case 'p':
				AddRandomPowerupToWorld(Vector3(nodePosition.x, nodePosition.y + 5, nodePosition.z));
				break;
			case 'd':
				AddCrateToWorld(Vector3(nodePosition.x, nodePosition.y + 5, nodePosition.z), (gridWidth * y) + x);
				break;
			}
			
		}
	}


	mazeGrid = new NavigationGrid("Maze.txt");

	Goose* goose = AddGooseToWorld(Vector3(worldGrid->GridWidth() * worldGrid->NodeSize() + mazeGrid->GridWidth() * mazeGrid->NodeSize() + 10.0f,
		0,
		worldGrid->GridWidth() * worldGrid->NodeSize() / 3.5f + 10.0f));



	GridNode* allMNodes = mazeGrid->AllNodes();
	int gridMWidth = mazeGrid->GridWidth();
	int gridMHeight = mazeGrid->GridHeight();
	for (int y = 0; y < gridMHeight; ++y) {
		for (int x = 0; x < gridMWidth; ++x) {
			GridNode& n = allMNodes[(gridMWidth * y) + x];
			n.position = Vector3(worldGrid->GridWidth() * worldGrid->NodeSize() + mazeGrid->GridWidth() * mazeGrid->NodeSize() + n.position.x,
											n.position.y, 
											worldGrid->GridWidth() * worldGrid->NodeSize() / 3.5f  + n.position.z);

			
			Vector3 nodePosition = n.position;
			GameObject* object;
			switch (n.type) {
			case '.':
				//clear area!
				break;
			case 'x':
				//wall
				object = AddCubeToWorld(Vector3(nodePosition.x, nodePosition.y + 12, nodePosition.z),
					Vector3(5, 10, 5), 0.0f);
				object->GetRenderObject()->SetColour(Vector4(0.07, 0.40, 0.00, 1.0f));
				break;
			case 'p':
				AddRandomPowerupToWorld(Vector3(nodePosition.x, nodePosition.y + 5, nodePosition.z));
				break;
			case 'd':
				Crate* crate =AddCrateToWorld(Vector3(nodePosition.x, nodePosition.y + 5, nodePosition.z), (gridMWidth * y) + x);
				goose->AddObjectToProtect(crate);
				break;
			}

		}
	}

	human->SetNavigationGrid(worldGrid);
	human->SetWorld(world);
	
	goose->SetNavigationGrid(mazeGrid);
	goose->SetWorld(world);

	if (player != nullptr)
		goose->SetPlayer(player);

	goose->xOffset = worldGrid->GridWidth() * worldGrid->NodeSize() + mazeGrid->GridWidth() * mazeGrid->NodeSize();
	goose->zOffset = worldGrid->GridWidth() * worldGrid->NodeSize() / 3.5f;

	int gridWidthLen = worldGrid->GridWidth() * worldGrid->NodeSize();
	int gridHeightLen = worldGrid->GridHeight() * worldGrid->NodeSize();
	Vector3 startPos = Vector3(gridWidthLen + 150, -10, gridHeightLen / 2);

	TriggerObject* trigger = AddTriggerToWorld(startPos, Vector3(5, 10, 10));
	trigger->SetObjectToEffect(goose);
	

	AddBridgeToWorld();
	InitFloors();
}

Bin* TutorialGame::AddBinToWorld(const Vector3& position)
{
	Bin* bin = new Bin(position);
	bin->SetRenderObject(new RenderObject(&bin->GetTransform(), cubeMesh, basicTex, basicShader));
	bin->GetRenderObject()->SetColour(Vector4(0.5, 0.5, 0.5, 1));
	world->AddGameObject(bin);
	return bin;
}



/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, bool hollow) {
	GameObject* sphere = new GameObject("sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	volume->SetDiscrete(false);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));
	sphere->GetPhysicsObject()->SetElasticity(0.2);
	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	if (hollow) {
		sphere->GetPhysicsObject()->InitHollowSphereIntertia();
	}
	else {
		sphere->GetPhysicsObject()->InitSphereInertia();
	}

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("cube");

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, nullptr, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

Player* TutorialGame::AddPlayerToWorld(const Vector3& position, int id) {
	Player* player = new Player(position);
	player->SetRenderObject(new RenderObject(&player->GetTransform(), charMesh, nullptr, basicShader));
	player->GetRenderObject()->SetColour(player->defaultColour);
	NetworkObject* n = new NetworkObject(*player, id);
	//player->SetNetworkObject(n);

	world->AddGameObject(player);
	player->SetWorldID(id);
	return player;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject("Enemy", enemy, enemyLayerMask);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject("apple");

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

Crate* TutorialGame::AddCrateToWorld(const Vector3& position, int nodeReference)
{
	Crate* crate = new Crate(position);
	crate->SetRenderObject(new RenderObject(&crate->GetTransform(), cubeMesh, nullptr, basicShader));
	crate->GetRenderObject()->SetColour(Vector4(0.5, 0.35, 0.05, 1));
	world->AddGameObject(crate);
	crate->nodeReference = nodeReference;

	destructableObjects.push_back(crate);
	return crate;
}

Human* TutorialGame::AddHumanToWorld(const Vector3& position)
{
	Human* human = new Human(position);
	human->SetRenderObject(new RenderObject(&human->GetTransform(), enemyMesh, nullptr, basicShader));
	human->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	world->AddGameObject(human);

	humanObjects.push_back(human);
	
	return human;
}

Powerup* TutorialGame::AddRandomPowerupToWorld(const Vector3& position)
{
	int random = rand() % 3;
	Powerup* randomPowerup = nullptr;
	switch (random) {
	case 0:
		randomPowerup = AddPowerupToWorld(position, Speed);
		break;
	case 1:
		randomPowerup = AddPowerupToWorld(position, Jump);
		break;
	case 2:
		randomPowerup = AddPowerupToWorld(position, Invisibility);
		break;
	}
	return randomPowerup;

}

Powerup* TutorialGame::AddPowerupToWorld(const Vector3& position, PowerupType powerupType)
{
	Powerup* powerup = new Powerup(position, powerupType);
	powerup->SetRenderObject(new RenderObject(&powerup->GetTransform(), sphereMesh, basicTex, basicShader));

	switch (powerupType) {
	case Speed:
		powerup->GetRenderObject()->SetColour(Vector4(0.0f, 1.0f, 0.0f, 1));
		break;
	case Jump:
		powerup->GetRenderObject()->SetColour(Vector4(0.0f, 0.0f, 1.0, 1));
		break;
	case Invisibility:
		powerup->GetRenderObject()->SetColour(Vector4(1.0f, 1.0f, 1.0f, 0.2f));
		break;

	}
	world->AddGameObject(powerup);

	powerupObjects.push_back(powerup);

	return powerup;
}

Goose* TutorialGame::AddGooseToWorld(const Vector3& position)
{
	Goose* goose = new Goose(position);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooshMesh, nullptr, basicShader));
	
	world->AddGameObject(goose);
	return goose;
}

TriggerObject* TutorialGame::AddTriggerToWorld(const Vector3& position, Vector3 dimensions)
{
	TriggerObject* trigger = new TriggerObject(position, dimensions);
	//trigger->SetRenderObject(new RenderObject(&trigger->GetTransform(), cubeMesh, nullptr, basicShader));

	world->AddGameObject(trigger);
	return trigger;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position)
{
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(3, 3, 3))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), enemyMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitFloors() {
	float worldWidth = worldGrid->GridWidth() * worldGrid->NodeSize();
	float worldHeight = worldGrid->GridHeight() * worldGrid->NodeSize();

	float mazeWidth = mazeGrid->GridWidth() * mazeGrid->NodeSize();
	float mazeHeight = mazeGrid->GridHeight() * mazeGrid->NodeSize();

	AddMainFloorToWorld(Vector3(worldWidth/ 2, -10, worldHeight/ 2));

	AddMazeFloorToWorld(Vector3(worldWidth + mazeWidth + 70,
		-10,
		mazeWidth));



}

GameObject* TutorialGame::AddMainFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("floor");
	int nodeSize = worldGrid->NodeSize();
	Vector3 floorSize = Vector3(worldGrid->GridWidth() * nodeSize / 2
		, 2, worldGrid->GridHeight() * nodeSize / 2);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddMazeFloorToWorld(const Vector3& position)
{
	GameObject* floor = new GameObject("floor");
	int nodeSize = mazeGrid->NodeSize();
	Vector3 floorSize = Vector3(mazeGrid->GridWidth() * nodeSize / 2
		, 2, mazeGrid->GridHeight() * nodeSize / 2);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

void TutorialGame::AddBridgeToWorld() {
	Vector3 cubeSize = Vector3(4, 1, 8);

	float invCubeMass = 0;//10;
	int numLinks = 10;
	float maxDistance = 11;
	float cubeDistance = 10;

	int gridWidth = worldGrid->GridWidth()* worldGrid->NodeSize();
	int gridHeight = worldGrid->GridHeight() * worldGrid->NodeSize();
	Vector3 startPos = Vector3(gridWidth + 10, -10, gridHeight/ 2);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	
	start->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* pConstraint = new PositionConstraint(previous, block, maxDistance);
		OrientationConstraint* oConstraintY = new OrientationConstraint(previous, block, Vector3(0, 1, 0));
		OrientationConstraint* oConstraintX = new OrientationConstraint(previous, block, Vector3(1, 0, 0));
		OrientationConstraint* oConstraintZ = new OrientationConstraint(previous, block, Vector3(0, 0, 1));
		world->AddConstraint(pConstraint);
		world->AddConstraint(oConstraintY);
		world->AddConstraint(oConstraintX);
		world->AddConstraint(oConstraintZ);
		previous = block;
	}

	PositionConstraint* pConstraint = new PositionConstraint(previous, end, maxDistance);
	OrientationConstraint* oConstraintY = new OrientationConstraint(previous, end, Vector3(0,1,0));
	OrientationConstraint* oConstraintX = new OrientationConstraint(previous, end, Vector3(1, 0, 0));
	OrientationConstraint* oConstraintZ = new OrientationConstraint(previous, end, Vector3(0, 0, 1));
	world->AddConstraint(pConstraint);
	world->AddConstraint(oConstraintY);
	world->AddConstraint(oConstraintX);
	world->AddConstraint(oConstraintZ);

}



/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}

	if (inSelectionMode) {
		Vector3 forward = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);
		Vector3 right = selectionObject->GetTransform().GetOrientation() * Vector3(1, 0, 0);
		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::W)) {
			selectionObject->GetPhysicsObject()->AddForceAtPosition(forward * forceMagnitude, selectionObject->GetTransform().GetPosition());
		}
		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::A)) {
			selectionObject->GetPhysicsObject()->AddForceAtPosition(-right * forceMagnitude, selectionObject->GetTransform().GetPosition());
		}
		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::S)) {
			selectionObject->GetPhysicsObject()->AddForceAtPosition(-forward * forceMagnitude, selectionObject->GetTransform().GetPosition());
		}
		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::D)) {
			selectionObject->GetPhysicsObject()->AddForceAtPosition(right * forceMagnitude, selectionObject->GetTransform().GetPosition());
		}

	}
}


