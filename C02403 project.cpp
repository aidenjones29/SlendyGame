// C02403 project.cpp: A program using the TL-Engine
#pragma once
#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h> // General console window includes
#include <conio.h>
#include <ctype.h>
#include <memory>
#include <time.h>
#include "ModelCreation.h"
#include "Collisions.h"
#include "wtypes.h" 
#include "Bullets.h"
#include "Weapons.h"
#include "Targets.h"
#include "Weapons.h"
#include "Engine.h"
//#include <list>

using namespace tle;

enum menuSelection { Play, Options, Controls, Quit};
enum standingState { Standing, Crouching, Prone };
enum menuState {MainMenu, PauseMenu, GameRunning, ControlsMenu};
enum gunState {NoGun, HoldingGun};
enum EReloadState {GunReloaded,GunReloading};

I3DEngine* myEngine = New3DEngine(kTLX);

const float UPPER_CAM_Y_MAX = -50.0f;
const float LOWER_CAM_Y_MAX = 50.0f;
const float RECOIL_SPEED = -50;
const float gunMovementMax = -3;

float Time = 0;
int finalTime = 0;
float targetTime[3] = { 0,0,0 };
bool runStarted = false;
int Score = 0;
int finalScore = 0;

float countDownTime = 1.0f;
float WeaponTime = 0;
int bulletsFired = 0;
int HitScore = 0;

bool canShoot = true;
bool recoil = false;
const float recoilLimit = 10.0f;
float currentRecoil = 0.0f;
const float recoilAmount = 6.0f;
float currentGunMoveBack = 0;

vector <sBullet*> vBullets;
vector <sBullet*> vMagazine;
vector <sTarget*> vTargets;
deque <unique_ptr<sWeapon>> vGuns;
sWeapon* currentGun = nullptr;
fireModes CurrentFireMode = Auto;

void Fire(IModel* &cameraDummy, float& frametime, float& shoottimer, float& camYCounter, sWeapon* &currentGun, bool& recoil);

void movement(I3DEngine* myEngine, IModel* camDummy, float& currentCamRotation, float& currentCamY, float& camYCounter, standingState& currPlayerStandState, float& movementSpeed, float& currentMoveSpeed);

void desktopResolution(int& horizontal, int& vertical);

void loadSounds();

struct Particle
{
	// Content to be added
	float positionX, positionY, positionZ;
	float velocityX, velocityY, velocityZ;
	float lifeTimer;
};

sf::SoundBuffer nickStartBuffer;
sf::SoundBuffer nickTimerBuffer;
sf::SoundBuffer nickWellDoneBuffer;
sf::SoundBuffer nickPatheticBuffer;
sf::SoundBuffer nickWhatBuffer;
sf::Sound nickWhatSound;
sf::Sound nickStartSound;
sf::Sound nickTimerSound;
sf::Sound nickWelldoneSound;
sf::Sound nickPatheticSound;
EReloadState ReloadState = GunReloaded;

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	int horizontal = 0; int vertical = 0;
	float reloadTimer = 0;
	desktopResolution(horizontal, vertical);
	myEngine->StartFullscreen(horizontal, vertical);
	myEngine->StartMouseCapture();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder(".\\Media");
	ICamera* myCam = myEngine->CreateCamera(kManual, 0, 0, 0);
	{
		IFont* MainFont = myEngine->LoadFont("Stencil STD", 60);
		
		ISprite* Crosshair = myEngine->CreateSprite("crosshair.png", (horizontal / 2) - 6000, (vertical / 2) - 60);
		ISprite* ammoUI = myEngine->CreateSprite("ammoUIPNG.png", -10000, vertical - 150);
		ISprite* menuUI = myEngine->CreateSprite("mainMenuUI.png", -10000, 0, 0.1);
		ISprite* controlsUI = myEngine->CreateSprite("controlsMenuUI.png", -100000, 0, 0.1);
		ISprite* menuSelectionUI = myEngine->CreateSprite("menuSelectionUISprite.png", 90, 425);
		ISprite* fireModeSemi = myEngine->CreateSprite("SemiAutoUI.png", -20, vertical - 105);
		ISprite* fireModeBurstUI = myEngine->CreateSprite("burstFireUI.png", -29, vertical - 105);
		ISprite* fireModeFullUI = myEngine->CreateSprite("FullAutoUI.png", -43, vertical - 105);

		IMesh* dummyMesh = myEngine->LoadMesh("Dummy.x");
		IMesh* bulletMesh = myEngine->LoadMesh("Bullet.x");
		IMesh* gateMesh = myEngine->LoadMesh("Gate.x");
		IMesh* particleMesh = myEngine->LoadMesh("Bullet.x");
		IMesh* muzzlemesh = myEngine->LoadMesh("muzzleFlash.x");

		IModel* fence[80];
		IModel* cameraDummy = dummyMesh->CreateModel(5, 15, 80);
		IModel* interactionDummy = dummyMesh->CreateModel(0, 0, 0);
		IModel* gunFireTest = dummyMesh->CreateModel(0, 0, 0);
		IModel* ammoCrate[numAmmoBoxes];
		IModel* gates[2];
		IModel* gateDummy[2];
		IModel* particle;
		loadSounds();
		SetupFlash(muzzlemesh);

		gateDummy[0] = dummyMesh->CreateModel(127, 9, 120);
		gates[0] = gateMesh->CreateModel(-6, 0, 0);
		gates[0]->AttachToParent(gateDummy[0]);

		gateDummy[1] = dummyMesh->CreateModel(172.8f, 9, 50.8);
		gates[1] = gateMesh->CreateModel(0, 0, 5.5);
		gates[1]->AttachToParent(gateDummy[1]);

		gates[1]->RotateY(90);
		gates[0]->ScaleZ(0.05f);
		gates[0]->ScaleX(1.15f);
		gates[0]->ScaleY(1.7f);
		gates[1]->ScaleZ(0.05f);
		gates[1]->ScaleX(1.15f);
		gates[1]->ScaleY(1.7f);

		menuState currentGameState = MainMenu;
		gunState currentGunState = NoGun;
		menuSelection currentMenuSelection = Play;
		standingState currPlayerStandState = Standing;
		
		stringstream ammoText;
		stringstream FPS;
		stringstream TimerCount;
		stringstream ScoreCounter;

		gunFireTest->AttachToParent(cameraDummy);

		spawnTargets(vTargets);
		spawnGuns(vGuns);
		spawnBullets(500, bulletMesh, vBullets);
		refillNewWeapon(100, vMagazine, vBullets);

		interactionDummy->Scale(7);
		interactionDummy->AttachToParent(myCam);

		myCam->AttachToParent(cameraDummy);
		myCam->SetMovementSpeed(0.0f);
		cameraDummy->RotateY(180);

		float frameTime = myEngine->Timer();
		float movementSpeed = frameTime;
		float shoottimer = 0.04f;
		float currentMoveSpeed = 50.0f;
		float mouseMoveX = 0.0f;
		float mouseMoveY = 0.0f;
		float camYCounter = 0.0f;
		float interactionZspeed = 0.0f;
		float currentInteractionDistance = 0.0f;
		float GunReloadTimer= 0.0f;

		bool gateOpen[2] = { false, false };
		bool gateHorizontal[2] = { true, false };
		bool canCollide = false;
		bool crouched = false;
		bool prone = false;
		bool spritesInPosition = false;
		bool nicktimerWillstartSaid = false;
		bool whatSaidOnce = false;
		float oldPlayerPos[2];

		/**** Set up your scene here ****/
		CreateFences(fence); CreateScene(ammoCrate); CreateWalls();

		// The main game loop, repeat until engine is stopped
		while (myEngine->IsRunning())
		{

			frameTime = myEngine->Timer();
			WeaponTime = WeaponTime + frameTime;
			myEngine->DrawScene();
            MuzzleFlash(cameraDummy);
			if (currentGun != nullptr)
			{
				ammoText << currentGun->magAmount << " / " << currentGun->magCapacity;
				MainFont->Draw(ammoText.str(), 180, vertical - 74, kWhite, kCentre);
				MainFont->Draw(currentGun->name, 180, vertical - 115, kWhite, kCentre);
				ammoText.str("");
			}

			oldPlayerPos[0] = cameraDummy->GetX();
			oldPlayerPos[1] = cameraDummy->GetZ();

			movementSpeed = currentMoveSpeed * frameTime;

			/**** Update your scene each frame here ****/
			//**************************************************** Game Start ********************************************************//
			if (currentGameState == MainMenu)
			{
				menuUI->SetPosition(0, 0);
				controlsUI->SetPosition(-100000, 0);

				if (myEngine->KeyHit(Key_Up))
				{
					if (currentMenuSelection == Play)
					{
						currentMenuSelection = Quit;
						menuSelectionUI->SetPosition(90, 810);
					}
					else if (currentMenuSelection == Options)
					{
						currentMenuSelection = Play;
						menuSelectionUI->SetPosition(90, 425);
					}
					else if (currentMenuSelection == Controls)
					{
						currentMenuSelection = Options;
						menuSelectionUI->SetPosition(90, 555);
					}
					else if(currentMenuSelection == Quit)
					{
						currentMenuSelection = Controls;
						menuSelectionUI->SetPosition(90, 680);
					}
				}

				if (myEngine->KeyHit(Key_Down))
				{
					if (currentMenuSelection == Play)
					{
						currentMenuSelection = Options;
						menuSelectionUI->SetPosition(90, 555);
					}
					else if (currentMenuSelection == Options)
					{
						currentMenuSelection = Controls;
						menuSelectionUI->SetPosition(90, 680);
					}
					else if (currentMenuSelection == Controls)
					{
						currentMenuSelection = Quit;
						menuSelectionUI->SetPosition(90, 810);
					}
					else if (currentMenuSelection == Quit)
					{
						currentMenuSelection = Play;
						menuSelectionUI->SetPosition(90, 425);
					}
				}

				if (myEngine->KeyHit(Key_Return))
				{
					if (currentMenuSelection == Play)
					{
						currentGameState = GameRunning;
					}
					else if (currentMenuSelection == Options)
					{
						currentGameState = GameRunning;
					}
					else if (currentMenuSelection == Controls)
					{
						currentGameState = ControlsMenu;
					}
					else if (currentMenuSelection == Quit)
					{
						myEngine->Stop();
					}
				}
			}
			//**************************************************** Controls Menu ********************************************************//
			else if (currentGameState == ControlsMenu)
			{
				menuUI->SetX(-10000);
				menuSelectionUI->SetX(-10000);
				controlsUI->SetPosition(0, 0);

				if (myEngine->KeyHit(Key_Return))
				{
					currentGameState = MainMenu;
					menuSelectionUI->SetPosition(90, 425);
					currentMenuSelection = Play;
				}
			}
			//**************************************************** Main game ********************************************************//
			else if (currentGameState == GameRunning)
			{
				if (!spritesInPosition)
				{
					nickStartSound.play();
					menuUI->SetX(-10000);
					menuSelectionUI->SetX(-10000);
					controlsUI->SetX(-10000);
					Crosshair->SetPosition((horizontal / 2) - 60, (vertical / 2) - 60);
					ammoUI->SetPosition(10, vertical - 150);
					spritesInPosition = true;
				}

				for (int i = 0; i < 3; i++)
				{
					if (vTargets[i]->state == Down)
					{
						targetTime[i] += frameTime;
					}

					if (targetTime[i] > 2)
					{
						vTargets[i]->model->SetY(12);
						vTargets[i]->state = Ready;
						targetTime[i] = 0;
					}
				}

				if (runStarted == false)
				{
					Score = finalScore;
					Time = finalTime;
				}

				if (CurrentFireMode == Auto)
				{
					fireModeBurstUI->SetPosition(29, vertical - 105);
					fireModeFullUI->SetPosition(43, vertical - 105);
					fireModeSemi->SetPosition(13, vertical - 105);
				}
				else if (CurrentFireMode == Burst)
				{
					fireModeFullUI->SetPosition(-28, vertical - 105);
					fireModeSemi->SetPosition(13, vertical - 105);
				}
				else
				{
					fireModeSemi->SetPosition(13, vertical - 105);
					fireModeFullUI->SetPosition(-28, vertical - 105);
					fireModeBurstUI->SetPosition(-28, vertical - 105);
				}

				mouseMoveX = myEngine->GetMouseMovementX();
				mouseMoveY = myEngine->GetMouseMovementY();

				if (camYCounter < UPPER_CAM_Y_MAX && mouseMoveY < 0) { mouseMoveY = 0; }
				if (camYCounter > LOWER_CAM_Y_MAX && mouseMoveY > 0) { mouseMoveY = 0; }

				camYCounter += mouseMoveY * 0.1f;

				movement(myEngine, cameraDummy, mouseMoveX, mouseMoveY, camYCounter, currPlayerStandState, movementSpeed, currentMoveSpeed);

				for (auto& Guns : vGuns)
				{
					if (canCollide == true && gunInteraction(interactionDummy, Guns->weaponModel) && currentGun == nullptr)
					{
						Guns->weaponModel->ResetOrientation();
						Guns->weaponModel->AttachToParent(cameraDummy);
						Guns->weaponModel->SetLocalPosition(2.0f, -2.0f, 7.0f);
						Guns->weaponModel->RotateY(-0.2f);
						currentGun = (Guns.get());
						reloadMagazine(currentGun->magCapacity, vMagazine);
						currentGun->magAmount = currentGun->magCapacity;

						if (nicktimerWillstartSaid == false)
						{
							nickTimerSound.play();
							nicktimerWillstartSaid = true;
						}
					}
				}

				if (targetBoxCollision(vTargets, cameraDummy, oldPlayerPos) == FrontBack || WallCollision(Walls, cameraDummy, oldPlayerPos) == FrontBack || allFenceCollisions(cameraDummy, oldPlayerPos) == FrontBack)
				{
					cameraDummy->SetZ(oldPlayerPos[1]);
				}
				
				if (targetBoxCollision(vTargets, cameraDummy, oldPlayerPos) == LeftRight || WallCollision(Walls, cameraDummy, oldPlayerPos) == LeftRight || allFenceCollisions(cameraDummy, oldPlayerPos) == LeftRight)
				{
					cameraDummy->SetX(oldPlayerPos[0]);
				}

				for (int i = 0; i < 2; i++)
				{
					if (gateOpen[i] == false && gateCollisions(gates[i], cameraDummy, oldPlayerPos, gateHorizontal[i]) == FrontBack)
					{
						cameraDummy->SetZ(oldPlayerPos[1]);
					}
					else if (gateOpen[i] == false && gateCollisions(gates[i], cameraDummy, oldPlayerPos, gateHorizontal[i]) == LeftRight)
					{
						cameraDummy->SetX(oldPlayerPos[0]);
					}
				}
				
				//------------------------------------ RUN STARTED ------------------------------------//

				if (canCollide == true && gunInteraction(interactionDummy, gates[0]))
				{
					if (gateOpen[0] == false && runStarted == false && currentGun != NULL)
					{
						gateDummy[0]->RotateY(120);
						gateOpen[0] = true;
						runStarted = true;
						Time = 0;
						Score = 0;
					}

					canCollide = false;
				}

				if (runStarted == true)
				{
					Time += frameTime;

					if (Time >= 2 && gateOpen[0] == true)
					{
						gateOpen[0] = false;
						gateDummy[0]->RotateY(-120);
					}

				}

				if (canCollide == true && gunInteraction(interactionDummy, gates[1]))
				{
					if (gateOpen[1] == false && runStarted == true)
					{
						gateDummy[1]->RotateY(-120);
						gateOpen[1] = true;
						runStarted = false;
						finalTime = int(Time);
						finalScore = Score;

						if (finalScore > 18)
						{
							nickWelldoneSound.play();
						}
						else
						{
							nickPatheticSound.play();
						}
					}

					canCollide = false;
				}

				if (myEngine->KeyHit(Key_E))
				{
					interactionZspeed = 0.0f;
					currentInteractionDistance = 0.0f;
					interactionDummy->SetLocalPosition(0, 0, 0);
					interactionZspeed = 0.1f;
					canCollide = true;
				}

				if (myEngine->KeyHit(Key_Q) && currentGun != nullptr)
				{
					currentGun->weaponModel->DetachFromParent();
					currentGun->weaponModel->SetPosition(oldPlayerPos[0], 0.2, oldPlayerPos[1]);
					currentGun->weaponModel->RotateLocalZ(90.0f);
					currentGun->weaponModel->RotateY(rand());
					currentGun = nullptr;
				}

				if (currentInteractionDistance >= 2.0f)
				{
					canCollide = false;
					interactionZspeed = 0.0f;
					currentInteractionDistance = 0.0f;
					interactionDummy->SetLocalPosition(0, 0, 0);
				}

				interactionDummy->MoveLocalZ(interactionZspeed);
				currentInteractionDistance += interactionZspeed;

				if (myEngine->KeyHit(Key_X) && currentGun != nullptr)
				{
					if (CurrentFireMode == Auto)
					{
						CurrentFireMode = Burst;
					}
					else if (CurrentFireMode == Burst)						
					{
						CurrentFireMode = Single;
					}
					else if (CurrentFireMode == Single)
					{
						CurrentFireMode = Auto;
					}
				}

				if (currentGun != nullptr)
				{
					if (currentGun->name == "Glock")
					{
						CurrentFireMode = Single;
					}
				}
			/*	if (myEngine->KeyHeld(Key_R) && currentGun != nullptr)
				{
					reloadMagazine(currentGun->magCapacity, vMagazine);
					currentGun->magAmount = currentGun->magCapacity;
				}*/


			if (myEngine->KeyHit(Key_R) && currentGun != nullptr)
				{
					ReloadState = GunReloading;
				}
				
				if (ReloadState == GunReloading)
				{
					if (GunReloadTimer > 1.5f)
					{
	                    reloadMagazine(currentGun->magCapacity, vMagazine);
						currentGun->magAmount = currentGun->magCapacity;
						ReloadState = GunReloaded;
						GunReloadTimer = 0.0f;
					}
					else
					{
						GunReloadTimer += frameTime;
					}
				}
			

				if (myEngine->KeyHit(Key_N) && runStarted == false)
				{
					for (auto& i : vTargets)
					{
						i->model->SetY(12);
						i->state = Ready;
					}

					for (int i = 0; i < 2; i++)
					{
						if (gateOpen[i] == true)
						{
							gateOpen[i] = false;
							gateDummy[i]->RotateY(120);
						}
					}
					Score = 0;
				}

				if (currentGun != nullptr)
				{
					Fire(cameraDummy, frameTime, shoottimer, camYCounter, currentGun, recoil);
				}

				if (recoil == true)
				{
					if (currentRecoil < 50)
					{
						currentGun->weaponModel->MoveLocalZ(-recoilAmount * frameTime);
						currentRecoil += recoilAmount * frameTime;
						recoil = false;
						GenerateMuzzleFlash(cameraDummy,currentGun->gunlength);
					}
				}
				else
				{
					if (currentRecoil > 0)
					{
						currentGun->weaponModel->MoveLocalZ(1 * frameTime);
						currentRecoil -= 1 * frameTime;
					}
				}

				moveBullets(100, vMagazine, frameTime);
				moveTargets(vTargets, frameTime);
				bulletToTarget(vTargets, vMagazine, Score, nickWhatSound, whatSaidOnce);
				bulletToWalls(Walls, vMagazine);

				FPS << "FPS: " << int(1 / frameTime);
				MainFont->Draw(FPS.str(), 20, 0, kWhite);
				FPS.str(" ");

				TimerCount << "Time: " << int(Time);
				MainFont->Draw(TimerCount.str(), horizontal - 250, 0, kWhite);
				TimerCount.str(" ");

				ScoreCounter << "Score: " << Score;
				MainFont->Draw(ScoreCounter.str(), horizontal / 2, 0, kWhite);
				ScoreCounter.str(" ");

				//END
			}

			if (myEngine->KeyHeld(Key_Escape))
			{
				myEngine->Stop();
			}

		}
		myEngine->Delete();	// Delete the 3D engine now we are finished with it
	}
}





void movement(I3DEngine* myEngine, IModel* camDummy, float& currentCamX, float &mouseMoveY, float& camYCounter, standingState& currPlayerStandState, float& movementSpeed, float& currentMoveSpeed)
{
	if (camYCounter > UPPER_CAM_Y_MAX && mouseMoveY < 0)
	{
		camDummy->RotateLocalX(mouseMoveY * 0.1f);
	}

	if (camYCounter < LOWER_CAM_Y_MAX && mouseMoveY > 0)
	{
		camDummy->RotateLocalX(mouseMoveY * 0.1f);
	}

	camDummy->RotateY(currentCamX * 0.1f);

	if (myEngine->KeyHeld(Key_W))
	{
		camDummy->MoveLocalZ(movementSpeed);
	}

	if (myEngine->KeyHeld(Key_S))
	{
		camDummy->MoveLocalZ(-movementSpeed);
	}

	if (myEngine->KeyHeld(Key_A))
	{
		camDummy->MoveLocalX(-movementSpeed);
	}

	if (myEngine->KeyHeld(Key_D))
	{
		camDummy->MoveLocalX(movementSpeed);
	}

	if (myEngine->KeyHit(Key_C))
	{
		if (currPlayerStandState == Standing)
		{
			currPlayerStandState = Crouching;
			currentMoveSpeed = 25.0f;
		}
		else if (currPlayerStandState == Crouching)
		{
			currPlayerStandState = Prone;
			currentMoveSpeed = 10.0f;
		}
		else if (currPlayerStandState == Prone)
		{
			currPlayerStandState = Standing;
			currentMoveSpeed = 50.0f;
		}
	}

	if (currPlayerStandState == Crouching)
	{
		camDummy->SetY(9);
	}
	else if (currPlayerStandState == Prone)
	{
		camDummy->SetY(3);
	}
	else if (currPlayerStandState == Standing)
	{
		camDummy->SetY(15);
	}
}

void desktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;						      //Gets a handle to the current window
	const HWND hDesktop = GetDesktopWindow(); //Gets the size and places it to a variable
	GetWindowRect(hDesktop, &desktop);        //Gets the coordinates for the corner of the screen

	horizontal = desktop.right;               //Holds the values for the screen resolution.
	vertical = desktop.bottom;				  //Holds the values for the screen resolution.
}

void Fire(IModel* &cameraDummy, float& frameTime, float& shoottimer, float& camYCounter, sWeapon* &currentGun, bool& recoil)
{
	int gunMoveAmount = 0;
	switch (CurrentFireMode)
	{
	case Auto:
		if (myEngine->KeyHeld(Mouse_LButton))
		{
			shoottimer -= frameTime;
			if (shoottimer <= 0 && currentGun->magAmount > 0)
			{
				currentGun->shootingSound.play();
				shoottimer = currentGun->fireRate * 2;
			}

			for (int i = 0; i <currentGun->magCapacity; i++)
			{
				if (WeaponTime >currentGun->fireRate)
				{
					if (vMagazine[i]->status == Reloaded)
					{
						float matrix[4][4];
						cameraDummy->GetMatrix(&matrix[0][0]);
						vMagazine[i]->model->SetMatrix(&matrix[0][0]);
						vMagazine[i]->model->MoveLocalZ(10.0f);
						vMagazine[i]->model->RotateLocalX(90.0f);
						vMagazine[i]->model->Scale(0.004f);
						vMagazine[i]->status = Fired;
						currentGun->magAmount--;
						WeaponTime = 0.0f;
						recoil = true;

						if (camYCounter > UPPER_CAM_Y_MAX)
						{
							for (int i = 0; i < 3; i++)
							{
								for (int j = 0; j < 10000; j++) {}
								cameraDummy->RotateLocalX(RECOIL_SPEED * frameTime);
								camYCounter += RECOIL_SPEED * frameTime;
							}
						}
					}
				}

			}
		}
		break;
	case Burst:
		
		if (myEngine->KeyHit(Mouse_LButton))
		{
			canShoot = false;
		}
		
		if (!canShoot)
		{
			for (int i = 0; i <currentGun->magCapacity; i++)
			{
				if (WeaponTime > (currentGun->fireRate / 2))
				{
					if (vMagazine[i]->status == Reloaded)
					{
						currentGun->shootingSound.play();
						float matrix[4][4];
						cameraDummy->GetMatrix(&matrix[0][0]);
						vMagazine[i]->model->SetMatrix(&matrix[0][0]);
						vMagazine[i]->model->MoveLocalZ(10.0f);
						vMagazine[i]->model->RotateLocalX(90.0f);
						vMagazine[i]->model->Scale(0.004f);
						vMagazine[i]->status = Fired;
						currentGun->magAmount--;
						WeaponTime = 0.0f;
						bulletsFired++;
						recoil = true;

						if (camYCounter > UPPER_CAM_Y_MAX)
						{
							for (int i = 0; i < 3; i++)
							{
								for (int j = 0; j < 10000; j++) {}
								cameraDummy->RotateLocalX(RECOIL_SPEED * frameTime);
								camYCounter += RECOIL_SPEED * frameTime;
							}
						}
					}

				}
			}
		}
		if (bulletsFired == 3)
		{
			canShoot = true;
			bulletsFired = 0;
		}
		break;
	case Single:
		if (myEngine->KeyHit(Mouse_LButton))
		{
			for (int i = 0; i < currentGun->magCapacity; i++)
			{
				if (vMagazine[i]->status == Reloaded)
				{
					currentGun->shootingSound.play();
					float matrix[4][4];
					cameraDummy->GetMatrix(&matrix[0][0]);
					vMagazine[i]->model->SetMatrix(&matrix[0][0]);
					vMagazine[i]->model->MoveLocalZ(10.0f);
					vMagazine[i]->model->RotateLocalX(90.0f);
					vMagazine[i]->model->Scale(0.004f);
					vMagazine[i]->status = Fired;
					currentGun->magAmount--;
					recoil = true;

					if (camYCounter > UPPER_CAM_Y_MAX)
					{
						for (int i = 0; i < 3; i++)
						{
							cameraDummy->RotateLocalX(RECOIL_SPEED * frameTime);
							camYCounter += RECOIL_SPEED * frameTime;
						}
					}

					break;
				}
			}
			break;
		}
	}
}

void loadSounds()
{
	nickStartBuffer.loadFromFile("soundeffects\\NICKwelcomeRecruit.wav");
	nickStartSound.setBuffer(nickStartBuffer);
	nickStartSound.setVolume(soundVolume);

	nickTimerBuffer.loadFromFile("soundeffects\\NICKtimerWillStart.wav");
	nickTimerSound.setBuffer(nickTimerBuffer);
	nickTimerSound.setVolume(soundVolume);

	nickWellDoneBuffer.loadFromFile("soundeffects\\NICKwellDone.wav");
	nickWelldoneSound.setBuffer(nickWellDoneBuffer);
	nickWelldoneSound.setVolume(soundVolume);

	nickPatheticBuffer.loadFromFile("soundeffects\\NICKthatWasPathetic.wav");
	nickPatheticSound.setBuffer(nickPatheticBuffer);
	nickPatheticSound.setVolume(soundVolume);
	
	nickWhatBuffer.loadFromFile("soundeffects\\NICKwhatAreYouDoing.wav");
	nickWhatSound.setBuffer(nickWhatBuffer);
	nickWhatSound.setVolume(soundVolume);
}
