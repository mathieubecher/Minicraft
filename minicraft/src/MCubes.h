#pragma once

#include "engine/render/renderer.h"
#include "engine/render/vbo.h"
#include "cube.h"
#include "atlasMap.h"
#include "engine/noise/perlin.h";
#include <chrono>
#include <ctime>   

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
	static const int CHUNK_HEIGHT = 64; ///< Taille d'un chunk en nombre de cubes (n*n*n)
	static const int PERLINBOTTONBASE = 64;
	static const int PERLINHEIGHTBASE = 64;
	static const int PERLINHEIGHTGROT = 80;

	static YPerlin Perlin;
	static YPerlin Perlin1;
	static YPerlin Perlin2;
	static YPerlin PerlinHeight;
	static YPerlin PerlinBottom;

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

				
				int height = PERLINBOTTOM +pow((perlin - 0.35f) * 3.04f, nivelageIndicator) * PERLINHEIGHT - _ZPos * CHUNK_HEIGHT;
				int eau = 64 - _ZPos * CHUNK_HEIGHT;
				int cubemax = ((height > eau) ? height : eau) + 1;
				for (int Z = 0; Z < ((CHUNK_HEIGHT>cubemax)?cubemax:CHUNK_HEIGHT); ++Z) {
					MCube * cube = &_Cubes[X][Y][Z];
					if (Z <= height) {
						if(Z == height) cube->setType(MCube::CUBE_HERBE);
						else if (Z > height - 4) {
							cube->setType(MCube::CUBE_TERRE);
						}
						else  {
							cube->setType(MCube::CUBE_PIERRE);
						}
					}
					else {
						cube->setType(MCube::CUBE_EAU);
					}
				}
			}

		}
		/*
		for (int X = 0; X < CHUNK_SIZE; ++X) {
			int x = _XPos * CHUNK_SIZE + X;

			for (int Y = 0; Y < CHUNK_SIZE; ++Y) {
				int y = _YPos * CHUNK_SIZE + Y;
				float mountainScore = 5; // Plus c'est bas, plus c'est montagneux [0.5, 5]
				float oceanScore = 0.5f;  // Plus c'est haut, plus il y'a d'océan [1, 3]

				float PERLINBOTTOM = pow((PerlinBottom.sample((float)x, (float)y, (float)0) - 0.3) * 3, oceanScore) * (float)PERLINBOTTONBASE;
				float PERLINHEIGHT = pow((PerlinHeight.sample((float)x, (float)y, (float)100) - 0.3) * 3 * 0.7f + 0.3f, mountainScore) * (float)PERLINHEIGHTBASE;
				float TERREHEIGHT = 3 + 3 * pow((PerlinHeight.sample((float)x, (float)y, (float)100) - 0.3) * 3 * 0.7f + 0.3f, mountainScore);
				int airdist = 0;
				for (int Z = CHUNK_HEIGHT - 1; Z >= 0; --Z) {
					int z = _ZPos * CHUNK_HEIGHT + Z;
					MCube * cube = &_Cubes[X][Y][Z];

					if (z < PERLINBOTTOM) cube->setType(MCube::CUBE_PIERRE);
					else if (z <= PERLINBOTTOM + PERLINHEIGHT) {
						float perlin =
							(Perlin.sample((float)x, (float)y, (float)z - PERLINBOTTOM) * 0.85f
								+ Perlin1.sample((float)x, (float)y, (float)z - PERLINBOTTOM) * 0.05f
								+ Perlin2.sample((float)x, (float)y, (float)z - PERLINBOTTOM) * 0.1f);
						float value = (1 - (z - PERLINBOTTOM) / PERLINHEIGHT);
						float percent = pow(abs(value - 0.5f) * 2, 1);
						float val = perlin * (1 - percent) + value * (percent);

						if (val > 0.5f)
						{
							if (Z < CHUNK_HEIGHT - TERREHEIGHT || airdist > 0) airdist++;
							else {
								int air = 1;
								while (air < TERREHEIGHT && airdist == 0) {
									float perlinnext =
										(Perlin.sample((float)x, (float)y, (float)z + airdist - PERLINBOTTOM) * 0.85f
											+ Perlin1.sample((float)x, (float)y, (float)z + airdist - PERLINBOTTOM) * 0.05f
											+ Perlin2.sample((float)x, (float)y, (float)z + airdist - PERLINBOTTOM) * 0.1f);
									float valuenext = (1 - (z + airdist - PERLINBOTTOM) / PERLINHEIGHT);
									float percentnext = pow(abs(value - 0.5f) * 2, 1);
									float valnext = perlin * (1 - percent) + value * (percent);
									if (valnext > 0.5f) airdist = air;
									else ++air;
								}
								airdist = air;
							}
							if (z > 46) {
								if (airdist <= 1)cube->setType(MCube::CUBE_HERBE);
								else if (airdist < TERREHEIGHT) cube->setType(MCube::CUBE_TERRE);
								else cube->setType(MCube::CUBE_PIERRE);
							}
							else {
								if (airdist <= 1)cube->setType(MCube::CUBE_SABLE_01);
								else if (airdist < TERREHEIGHT) cube->setType(MCube::CUBE_SABLE_02);
								else cube->setType(MCube::CUBE_PIERRE);
							}
						}
						else airdist = 0;
					}
					if (z <= 45 && !cube->isPickable())
						cube->setType(MCube::CUBE_EAU);


				}
			}
		}
		*/

		// auto end = std::chrono::system_clock::now();
		// std::chrono::duration<double> total = end - start;
		// cout << "Generation du chunk - total : " << total.count() * 1000 << "ms"<<endl;
	}

};

YPerlin MCubes::Perlin = YPerlin();
YPerlin MCubes::Perlin1 = YPerlin();
YPerlin MCubes::Perlin2 = YPerlin();
YPerlin MCubes::PerlinBottom = YPerlin();
YPerlin MCubes::PerlinHeight = YPerlin();