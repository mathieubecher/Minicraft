#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "engine/utils/timer.h"
#include "world.h"
#include "Inputs.h"

class MAvatar
{
public:
	YVec3f Position;
	YVec3f Speed;
	static float gravity;
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
	float mass = 1;
	float movespeed = 10;
	float maxspeed = 300;
	bool fps = true;

	YCamera * Cam;
	MWorld * World;
	Inputs * inputs;

	YTimer _TimerStanding;

	MAvatar(YCamera * cam, MWorld * world, Inputs * inputs, float mass = 1)
	{
		YVec3f Speed = YVec3f(0, 0, 0);
		this->mass = mass;
		Position = cam->Position;
		this->inputs = inputs;
		Height = 1.8f;
		CurrentHeight = Height;
		Width = 0.8f;
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
		bool lastwater = InWater;
		InWater = World->getCube((int)floor(Position.X), (int)floor(Position.Y), (int)floor(Position.Z))->getType() == MCube::CUBE_EAU;
		lastwater = InWater != lastwater;
		avance = inputs->Z.press;
		recule = inputs->S.press;
		droite = inputs->D.press;
		gauche = inputs->Q.press;
		Jump = inputs->Space.press;
		Run = inputs->Shift.press;
		

		float maxSpeed = 5;
		if (Run) maxSpeed = 9;
		if (InWater)maxSpeed = maxSpeed / 2.0f;
		float acc = maxSpeed * 10;
		if (!Standing && !InWater) acc = maxSpeed;


		YVec3f force = YVec3f(0, 0, 0);
		if (avance) force += Cam->Direction;
		if (recule) force -= Cam->Direction;
		if (droite) force -= Cam->RightVec;
		if (gauche) force += Cam->RightVec;
		force.Z = 0;
		force = force.normalize();
		force *= acc;
		
		if (Jump && ((Standing && !InWater) || lastwater)) {
			Standing = false;
			Speed.Z +=8.5f;
			Speed.X = force.normalize().X * maxSpeed;
			Speed.Y = force.normalize().Y * maxSpeed;
		}

		Speed += force * elapsed;
		float speedheight = Speed.Z;
		Speed.Z = 0;
		if (Speed.getSize() > maxSpeed) Speed = Speed.normalize() * maxSpeed;
		Speed.Z = (InWater)? ((Jump)?1:-1) : speedheight - gravity * elapsed;
		Position += (Speed * elapsed);

		//YLog::log(YLog::ENGINE_INFO, ("zS " + toString(Speed.Z)).c_str());

		if (_TimerStanding.getElapsedSeconds() > 0.01)
			Standing = false;
		for (int pass = 0; pass < 2; pass++)
		{
			for (int i = 0; i < 6; i++)
			{
				float valueColMin = 0;
				MWorld::MAxis axis = World->getMinCol(Position, Speed, Width, CurrentHeight, valueColMin, i < 3);
				//YLog::log(YLog::ENGINE_INFO,"check");
				if (axis != 0)
				{
					//valueColMin = nymax(nyabs(valueColMin), 0.0001f) * (valueColMin > 0 ? 1.0f : -1.0f);
					if (axis & MWorld::AXIS_X)
					{
						//YLog::log(YLog::ENGINE_INFO,("x " + toString(valueColMin)).c_str());
						Position.X += valueColMin + 0.001*sign(valueColMin);
						Speed.X = 0;
					}
					if (axis & MWorld::AXIS_Y)
					{
						//YLog::log(YLog::ENGINE_INFO, ("y " + toString(valueColMin)).c_str());
						Position.Y += valueColMin + 0.001*sign(valueColMin);
						Speed.Y = 0;
					}
					if (axis & MWorld::AXIS_Z)
					{
						//YLog::log(YLog::ENGINE_INFO, ("z " + toString(valueColMin)).c_str());
						Speed.Z = 0;
						Position.Z += valueColMin + 0.001*sign(valueColMin);
						Standing = true;
						_TimerStanding.start();
					}
				}
			}
		}
		if (InWater) {
			Speed *= pow(0.02f, elapsed);
		}
		else if (Standing) {
			Speed *= pow(0.01f, elapsed);
		}


		if(fps) Cam->setPosition(Position + YVec3f(0,0,0.6f));
		else {
			Cam->setPosition(Position - Cam->Direction * 5);
			Cam->setLookAt(Position);
		}

	}
};
float MAvatar::gravity = 9.81f * 3;
#endif