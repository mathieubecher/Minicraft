#pragma once

#include "engine/render/renderer.h"
#include "engine/render/vbo.h"
#include "cube.h"
#include "atlasMap.h"
#include "engine/noise/perlin.h";
#include <chrono>
#include <ctime>   
#include "smartPerlin.h"

class MCubesClear
{
public:
	MCubesClear() {

	}
	virtual inline MCube * get(int x, int y, int z) {
		return &MCube::Air;
	}
	virtual void generate(int _XPos, int _YPos, int _ZPos) {

	}
};

class MCubes : public MCubesClear
{
public:
	static const int CHUNK_SIZE = 32; ///< Taille d'un chunk en nombre de cubes (n*n*n)
	static const int CHUNK_HEIGHT = 32; ///< Taille d'un chunk en nombre de cubes (n*n*n)
	static const int PERLINBOTTONBASE = 64;
	static const int PERLINHEIGHTBASE = 64;
	static const int PERLINHEIGHTGROT = 80;

	static YSmartPerlin Perlin;

	MCube _Cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_HEIGHT]; ///< Cubes contenus dans le chunk
	MCubes()
	{
	}
	~MCubes(){
		delete [] _Cubes;
	}

	inline MCube * get(int x, int y, int z) {
		return &_Cubes[x][y][z];
	}

	virtual void generate(int _XPos, int _YPos, int _ZPos) {
		for (int X = 0; X < CHUNK_SIZE; ++X) {
			int x = _XPos * CHUNK_SIZE + X;

			for (int Y = 0; Y < CHUNK_SIZE; ++Y) {
				int y = _YPos * CHUNK_SIZE + Y;
				int airlevel = 0;
				bool water = false;
				for (int Z = CHUNK_HEIGHT-1; Z >= 0; --Z) {
					int z = _ZPos * CHUNK_HEIGHT + Z;
					float perlin = (Perlin.sample((float)x, (float)y, (float)z));
					
					if (Z == CHUNK_HEIGHT - 1 && perlin <= 0.5f) {
						bool isAir = false;
						while (airlevel < 5 && !isAir) {
							++airlevel;
							isAir = Perlin.sample((float)x, (float)y, (float)z+airlevel) > 0.5;
							
						}
					}
					
					water = (z + airlevel < 68);
					MCube *cube =  &_Cubes[X][Y][Z];

					if (perlin > 0.5)
					{
						if (z < 68) water = true;
						if (z < 66) {
							cube->setType(MCube::CUBE_EAU);
							
						}
						airlevel = 0;
					}
					else
					{
						++airlevel;
						if(perlin < 0.2) cube->setType(MCube::CUBE_PIERRE);
						else if(airlevel == 1 && !water) cube->setType(MCube::CUBE_HERBE);
						else if (airlevel < 5 && !water) cube->setType(MCube::CUBE_TERRE);
						else if (airlevel < 3 && water) cube->setType(MCube::CUBE_SABLE_01);
						else cube->setType(MCube::CUBE_PIERRE);
					}
						
				}
			}
		}
	}

};

YSmartPerlin MCubes::Perlin = YSmartPerlin();