#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "engine/utils/timer.h"
#include "world.h"

class MAvatar
{
public:
	YVec3f Position;
	YVec3f Speed;

	bool Move;
	bool Jump;
	float Height;
	float CurrentHeight;
	float Width;
	bool avance;
	bool recule;
	bool gauche;
	bool droite;
	bool Standing;
	bool InWater;
	bool Crouch;
	bool Run;

	YCamera * Cam;
	MWorld * World;

	YTimer _TimerStanding;

	MAvatar(YCamera * cam, MWorld * world)
	{
		Position = YVec3f((MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_HEIGHT_METERS));
		Height = 1.8f;
		CurrentHeight = Height;
		Width = 0.3f;
		Cam = cam;
		avance = false;
		recule = false;
		gauche = false;
		droite = false;
		Standing = false;
		Jump = false;
		World = world;
		InWater = false;
		Crouch = false;
		Run = false;
	}

	void update(float elapsed)
	{
		if (elapsed > 1.0f / 60.0f)
			elapsed = 1.0f / 60.0f;
	}
};

#endif