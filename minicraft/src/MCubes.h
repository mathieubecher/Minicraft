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
	static const int CHUNK_SIZE = 64; ///< Taille d'un chunk en nombre de cubes (n*n*n)
	static const int CHUNK_HEIGHT = 124; ///< Taille d'un chunk en nombre de cubes (n*n*n)
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
		// auto start = std::chrono::system_clock::now();
		/*
		float mountainIndicator = 0.5f;
		float oceanIndicator = 1;
		float nivelageIndicator = 2;

		for (int X = 0; X < CHUNK_SIZE; ++X) {
			int x = _XPos * CHUNK_SIZE + X;

			for (int Y = 0; Y < CHUNK_SIZE; ++Y) {
				int y = _YPos * CHUNK_SIZE + Y;
				float PERLINBOTTOM = PERLINBOTTONBASE * (pow((PerlinBottom.sample((float)x, (float)y, PERLINBOTTONBASE) - 0.35f) * 3.04f, oceanIndicator)*0.5f + 0.5f);
				float PERLINHEIGHT = PERLINHEIGHTBASE * (pow((PerlinHeight.sample((float)x, (float)y, PERLINBOTTONBASE) - 0.35f) * 3.04f, mountainIndicator));

				float perlin =
					(Perlin.sample((float)x, (float)y, PERLINBOTTOM) * 0.85f
						+ Perlin1.sample((float)x, (float)y, PERLINBOTTOM) * 0.05f
						+ Perlin2.sample((float)x, (float)y, PERLINBOTTOM) * 0.1f);


				int height = PERLINBOTTOM + pow((perlin - 0.35f) * 3.04f, nivelageIndicator) * PERLINHEIGHT - _ZPos * CHUNK_HEIGHT;
				int eau = 64 - _ZPos * CHUNK_HEIGHT;
				int cubemax = ((height > eau) ? height : eau);
				for (int Z = 0; Z < ((CHUNK_HEIGHT > cubemax) ? cubemax : CHUNK_HEIGHT); ++Z) {
					MCube * cube = &_Cubes[X][Y][Z];
					if (Z <= height - 1) {
						if (Z == height - 1) cube->setType(MCube::CUBE_HERBE);
						else if (Z > height - 5) {
							cube->setType(MCube::CUBE_TERRE);
						}
						else {
							cube->setType(MCube::CUBE_PIERRE);
						}
					}
					else {
						cube->setType(MCube::CUBE_EAU);
					}
				}
			}
		}
		*/

		// auto end = std::chrono::system_clock::now();
		// std::chrono::duration<double> total = end - start;
		// cout << "Generation du chunk - total : " << total.count() * 1000 << "ms"<<endl;
		for (int X = 0; X < CHUNK_SIZE; ++X) {
			int x = _XPos * CHUNK_SIZE + X;

			for (int Y = 0; Y < CHUNK_SIZE; ++Y) {
				int y = _YPos * CHUNK_SIZE + Y;
				int airlevel = 0;
				for (int Z = CHUNK_HEIGHT-1; Z >= 0; --Z) {
					int z = _ZPos * CHUNK_HEIGHT + Z;
					float perlin = (Perlin.sample((float)x, (float)y, (float)z));
					MCube *cube =  &_Cubes[X][Y][Z];

					if (perlin > 0.5)
					{
						//if(z<64) cube->setType(MCube::CUBE_EAU);
						airlevel = 0;
					}
					else
					{
						++airlevel;
						if(airlevel == 1) cube->setType(MCube::CUBE_HERBE);
						else if (airlevel < 5) cube->setType(MCube::CUBE_TERRE);
						else cube->setType(MCube::CUBE_PIERRE);
					}
						
				}
			}
		}
	}

};

YSmartPerlin MCubes::Perlin = YSmartPerlin();