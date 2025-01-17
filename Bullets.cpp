#pragma once
#include "Bullets.h"



vector<sMuzzleFlash*> vFlash;
sMuzzleFlash* flash = new sMuzzleFlash;
void spawnBullets(int maxBullets,IMesh* bulletMesh, vector<sBullet*> &vBullets)
{
	for (int i = 0; i < maxBullets; i++)
	{
		sBullet* bullet = new sBullet;
		bullet->model = bulletMesh->CreateModel(0, -30, 0);
		vBullets.push_back(bullet);
	}
}

void refillNewWeapon(int magSize,vector<sBullet*> &vMagazine,vector<sBullet*> &vBullets)
{
	vMagazine.resize(magSize);
	for (int i = 0; i < magSize; i++)
	{
		vMagazine[i] = move(vBullets[i]);
	}
}

void moveBullets(int magSize,vector<sBullet*> &vMagazine,float frameTime) 
{
	for (int i=0; i < magSize;i++) 
	{
		if (vMagazine[i]->status == Fired)
		{ 
 			vMagazine[i]->model->MoveLocalY(100000.0f*frameTime);
			vMagazine[i]->timeAlive = vMagazine[i]->timeAlive + frameTime;
		}
		if (vMagazine[i]->timeAlive > 4.0f && vMagazine[i]->status != Spent)
		{
			vMagazine[i]->status = Spent;
			vMagazine[i]->model->Scale(2);
			vMagazine[i]->model->SetY(0.2f);
		}
	}
}
void reloadMagazine(int magSize, vector<sBullet*> &vMagazine)
{

	for (int i = 0; i < magSize; i++)
	{
		vMagazine[i]->status = Reloaded;
		vMagazine[i]->model->SetPosition(0.0f, -25.0f, 0.0f);
		vMagazine[i]->timeAlive = 0.0f;
	}
}
void SetupFlash(IMesh* muzzleMesh) {
	flash->model = muzzleMesh->CreateModel(0, -30, 0);
	flash->model->SetSkin("muzzleFlash.png");
	flash->model->Scale(0.5f);
}
void GenerateMuzzleFlash(IModel* &cameraDummy ,float currentgunlength)
{
//float matrix[4][4];
//cameraDummy->GetMatrix(&matrix[0][0]);
//flash->model->SetMatrix(&matrix[0][0]);
//flash->model->MoveLocalZ(30.0f);
//flash->model->MoveLocalX(5.0f);
//flash->model->MoveLocalY(-2.0f);
	flash->model->ResetOrientation();
	flash->model->AttachToParent(cameraDummy);
	//mp4 33
	flash->model->SetLocalPosition(5.0f, -3.0f, currentgunlength);
}

void MuzzleFlash(IModel* &cameraDummy)
{
	flash->model->DetachFromParent();
	flash->model->SetY(-50);	

	
}