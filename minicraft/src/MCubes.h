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
	static const int PERLINBOTTONBASE = 64;
	static const int PERLINHEIGHTBASE = 64;

	static YPerlin Perlin;
	static YPerlin Perlin1;
	static YPerlin Perlin2;
	static YPerlin PHeight;
	static YPerlin PBottom;

	MCube _Cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; ///< Cubes contenus dans le chunk
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
		//YLog::log(YLog::USER_INFO, (toString("Creation du chunk ") + toString(chunk->_XPos) + toString(chunk->_YPos) + toString(chunk->_ZPos)).c_str());
		for (int X = 0; X < MCubes::CHUNK_SIZE; ++X) {
			int x = _XPos * MCubes::CHUNK_SIZE + X;

			for (int Y = 0; Y < MCubes::CHUNK_SIZE; ++Y) {
				int y = _YPos * MCubes::CHUNK_SIZE + Y;
				float mountainScore = 5; // Plus c'est bas, plus c'est montagneux [0.5, 5]
				float oceanScore = 1;  // Plus c'est haut, plus il y'a d'océan [1, 3]

				float PERLINBOTTOM = pow((PBottom.sample((float)x, (float)y, (float)0) - 0.3) * 3, oceanScore) * (float)PERLINBOTTONBASE;
				float PERLINHEIGHT = pow((PHeight.sample((float)x, (float)y, (float)100) - 0.3) * 3 * 0.7f + 0.3f, mountainScore) * (float)PERLINHEIGHTBASE;
				for (int Z = 0; Z < MCubes::CHUNK_SIZE; ++Z) {
					int z = _ZPos * MCubes::CHUNK_SIZE + Z;
					MCube * cube = get(X, Y, Z);
					if (z < PERLINBOTTOM) cube->setType(MCube::CUBE_PIERRE);
					else if (z <= PERLINBOTTOM + PERLINHEIGHT) {
						float perlin =
							(Perlin.sample((float)x, (float)y, (float)z - PERLINBOTTOM) * 0.85f
								+ Perlin1.sample((float)x, (float)y, (float)z - PERLINBOTTOM) * 0.05f
								+ Perlin2.sample((float)x, (float)y, (float)z - PERLINBOTTOM) * 0.1f);
						float value = (1 - (z - PERLINBOTTOM) / PERLINHEIGHT);
						float percent = pow(abs(value - 0.5f) * 2, 1);
						float val = perlin * (1 - percent) + value * (percent);

						if (val > 0.5f) cube->setType(MCube::CUBE_PIERRE);
					}
					if (z <= 45 && !cube->isPickable())
						cube->setType(MCube::CUBE_EAU);
				}
			}
		}

		// auto end = std::chrono::system_clock::now();
		// std::chrono::duration<double> total = end - start;
		// cout << "Generation du chunk - total : " << total.count() * 1000 << "ms"<<endl;
	}


};

YPerlin MCubes::Perlin = YPerlin();
YPerlin MCubes::Perlin1 = YPerlin();
YPerlin MCubes::Perlin2 = YPerlin();
YPerlin MCubes::PHeight = YPerlin();
YPerlin MCubes::PBottom = YPerlin();
