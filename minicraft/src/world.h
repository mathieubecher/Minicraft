#ifndef __WORLD_H__
#define __WORLD_H__

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "cube.h"
#include "chunk.h"
#include "engine/noise/perlin.h";
#include "engine_minicraft.h";

#include <vector>;
#include <list>;
#include <queue>;
#include <thread>;
#include <iostream>;
#include <mutex>;


class MWorld
{
public:

	typedef uint8 MAxis;
	static const int AXIS_X = 0b00000001;
	static const int AXIS_Y = 0b00000010;
	static const int AXIS_Z = 0b00000100;

#ifdef _DEBUG
	static const int MAT_SIZE = 400; //en nombre de chunks
#else
	static const int MAT_SIZE = 3; //en nombre de chunks
#endif // DEBUG

	static const int MAT_HEIGHT = 3; //en nombre de chunks
	static const int MAT_SIZE_CUBES = (MAT_SIZE * MCubes::CHUNK_SIZE);
	static const int MAT_HEIGHT_CUBES = (MAT_HEIGHT * MCubes::CHUNK_SIZE);
	static const int MAT_SIZE_METERS = (MAT_SIZE * MCubes::CHUNK_SIZE * MCube::CUBE_SIZE);
	static const int MAT_HEIGHT_METERS = (MAT_HEIGHT * MCubes::CHUNK_SIZE  * MCube::CUBE_SIZE);

	static const int PERLINBOTTONBASE = 40;
	static const int PERLINHEIGHTBASE = 100;
	static const int RADIUS = 2;
	static const int RADIUSDRAW = 5;

	std::vector<MChunk *> listChunks;

	std::list<YVec3<int>> neighbours;
	std::list<YVec3<int>> treats;

	std::queue<YVec3<int>> toLoads;
	std::queue<MChunk *> toVBOs;

	MChunk * actualChunk;

	MWorld()
	{

	}

	// Chargement des perlins
	void init_world(int seed)
	{

		//YLog::log(YLog::USER_INFO, (toString("Creation du monde seed ") + toString(seed)).c_str());

		srand(seed);

		MCubes::Perlin.setFreq(0.02f);
		MCubes::Perlin1.setFreq(0.04f);
		MCubes::Perlin2.setFreq(0.2f);

		MCubes::PBottom.setFreq(0.003f);
		MCubes::PHeight.setFreq(0.008f);

	}
	// Récupération du cube xyz
	inline MCube * getCube(int x, int y, int z)
	{
		int Xchunk = (int)floor(x / MCubes::CHUNK_SIZE), Ychunk = (int)floor(y / MCubes::CHUNK_SIZE), Zchunk = (int)floor(z / MCubes::CHUNK_SIZE);
		for (auto chunk : listChunks) {
			if (Xchunk == chunk->_XPos && Ychunk == chunk->_YPos && Zchunk == chunk->_ZPos) return chunk->_Cubes->get(x % MCubes::CHUNK_SIZE,y % MCubes::CHUNK_SIZE,z % MCubes::CHUNK_SIZE);
		}
		return &MCube::Air;
	}


	void updateCube(int x, int y, int z)
	{
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		if (z < 0)z = 0;
		if (x >= MAT_SIZE * MCubes::CHUNK_SIZE)x = (MAT_SIZE * MCubes::CHUNK_SIZE) - 1;
		if (y >= MAT_SIZE * MCubes::CHUNK_SIZE)y = (MAT_SIZE * MCubes::CHUNK_SIZE) - 1;
		if (z >= MAT_HEIGHT * MCubes::CHUNK_SIZE)z = (MAT_HEIGHT * MCubes::CHUNK_SIZE) - 1; {
			// Chunks[x / MCubes::CHUNK_SIZE][y / MCubes::CHUNK_SIZE][z / MCubes::CHUNK_SIZE]->disableHiddenCubes();
			// Chunks[x / MCubes::CHUNK_SIZE][y / MCubes::CHUNK_SIZE][z / MCubes::CHUNK_SIZE]->toVbos();
		}

	}

	void deleteCube(int x, int y, int z)
	{
		MCube * cube = getCube(x, y, z);
		if(cube->isPickable()){
			cube->setType(MCube::CUBE_AIR);
			cube->setDraw(false);
			cube = getCube(x - 1, y, z);
			updateCube(x, y, z);
		}
	}


	std::thread loadder;
	std::thread physicer;
	bool deleteLoadder = false;
	~MWorld() {
		deleteLoadder = true;
		loadder.join();
		physicer.join();
	}

	std::mutex lock;
	YCamera * mainCamera;
	bool firstCast = true;
	// INITIALISE LES CHUNKS AUTOUR DE LA CAMERA
	void initCam(YCamera * camera) {
		mainCamera = camera;
		YVec3f campos = mainCamera->Position;
		camChunk = YVec3<int>((int)floor(campos.X / MCubes::CHUNK_SIZE), (int)floor(campos.Y / MCubes::CHUNK_SIZE), (int)floor(campos.Z / MCubes::CHUNK_SIZE));
		addChunk((int)floor(campos.X / MCubes::CHUNK_SIZE), (int)floor(campos.Y / MCubes::CHUNK_SIZE), (int)floor(campos.Z / MCubes::CHUNK_SIZE));
		loadNearChunk();
		firstCast = false;
		updateCampos = true;
		// DEBUT DU THREAD QUI GERE LES CHUNK
		loadder = std::thread([this]() {
			while (!deleteLoadder) {
				if (updateCampos) {
					
					updateCampos = false;
					if (!actualChunk ) {
						addChunk(camChunk.X, camChunk.Y, camChunk.Z);
					}
					loadNearChunk();
				}
			}
		});
		// GERE LA PHYSIQUE DES CHUNK VISIBLE
		physicer = std::thread([this]() {
			
			while (!deleteLoadder) {
				
				int i = 0;
				while (i < listChunks.size()) {
					
					if (YVec3f(camChunk.X-listChunks[i]->_XPos, camChunk.Y - listChunks[i]->_YPos, camChunk.Z - listChunks[i]->_ZPos).getSize() > RADIUSDRAW) {
						MChunk *chunk = listChunks[i];
						auto it = listChunks.begin();
						for (int j = 0; j < i; ++j) ++it;
						lock.lock();
						listChunks.erase(it);
						lockNeighbour.lock();
						neighbours.push_back(YVec3<int>(chunk->_XPos, chunk->_YPos, chunk->_ZPos));
						lockNeighbour.unlock();
						delete chunk;
						lock.unlock();
					}
					else {
					
						lock.lock();
						MChunk * chunk = listChunks[i];
						lock.unlock();
						if (chunk->vbo && chunk->physic && YVec3f(camChunk.X - chunk->_XPos, camChunk.Y - chunk->_YPos, camChunk.Z - chunk->_ZPos).getSize() > RADIUS) {
							chunk->deleteCubes();
						}
					
						
						else if (chunk->vbo && !chunk->physic && YVec3f(camChunk.X - chunk->_XPos, camChunk.Y - chunk->_YPos, camChunk.Z - chunk->_ZPos).getSize() <= RADIUS) {
							chunk->reloadCubes();
							chunk->generate();
							chunk->disableHiddenCubes();
						}
						
						++i;
					
					}
				}
				
			}
			
		});

	}

	YVec3<int> camChunk;
	bool updateCampos = false;

	// ADAPTE LENVIRONNEMENT A LA NOUVELLE POSITION DE LA CAMERA
	void updateCam() {
		YVec3f campos = mainCamera->Position;
		camChunk = YVec3<int>((int)floor(campos.X / MCubes::CHUNK_SIZE), (int)floor(campos.Y / MCubes::CHUNK_SIZE), (int)floor(campos.Z / MCubes::CHUNK_SIZE));
		updateCampos |= refreshActualChunk(campos);
	

	}
	void LoadVBO() {
		if (toVBOs.size() > 0) {
			loadNewVBO();
		}
	}
	void loadNewVBO() {
		MChunk * chunk = toVBOs.front();
		toVBOs.pop();
		chunk->CreateVboGpu();
		chunk->draw = true;
	}

	// VERIFIE SI LA CAMERA A CHANGER DE CHUNK
	bool refreshActualChunk(YVec3f campos) {
		bool findactual = false;
		if (campos.X >= 0 && campos.X < MCubes::CHUNK_SIZE * MAT_SIZE &&
			campos.Y >= 0 && campos.Y < MCubes::CHUNK_SIZE * MAT_SIZE &&
			campos.Z >= 0 && campos.Z < MCubes::CHUNK_SIZE * MAT_HEIGHT)
		{
			while(actualChunk && !(campos.X >= actualChunk->_XPos * MCubes::CHUNK_SIZE  && campos.X < (actualChunk->_XPos + 1) * MCubes::CHUNK_SIZE &&
				campos.Y >= actualChunk->_YPos * MCubes::CHUNK_SIZE  && campos.Y < (actualChunk->_YPos + 1) * MCubes::CHUNK_SIZE &&
				campos.Z >= actualChunk->_ZPos * MCubes::CHUNK_SIZE  && campos.Z < (actualChunk->_ZPos + 1) * MCubes::CHUNK_SIZE)){
				if ((int)floor(campos.X / MCubes::CHUNK_SIZE) < actualChunk->_XPos) {
					actualChunk = actualChunk->Voisins[MChunk::Voisin::XPREV];
					findactual = true;
				}
				else if ((int)floor(campos.Y / MCubes::CHUNK_SIZE) < actualChunk->_YPos) {
					actualChunk = actualChunk->Voisins[MChunk::Voisin::YPREV];
					findactual = true;
				}
				else if ((int)floor(campos.Z / MCubes::CHUNK_SIZE) < actualChunk->_ZPos) {
					actualChunk = actualChunk->Voisins[MChunk::Voisin::ZPREV];
					findactual = true;
				}
				else if ((int)floor(campos.X / MCubes::CHUNK_SIZE) > actualChunk->_XPos) {
					actualChunk = actualChunk->Voisins[MChunk::Voisin::XNEXT];
					findactual = true;
				}
				else if ((int)floor(campos.Y / MCubes::CHUNK_SIZE) > actualChunk->_YPos) {
					actualChunk = actualChunk->Voisins[MChunk::Voisin::YNEXT];
					findactual = true;
				}
				else if ((int)floor(campos.Z / MCubes::CHUNK_SIZE) > actualChunk->_ZPos) {
					actualChunk = actualChunk->Voisins[MChunk::Voisin::ZNEXT];
					findactual = true;
				}
			}
			return findactual;
		} return false;
	}

	mutex lockNeighbour;
	// GENERATION DES VOISINS PAS ENCORE CHARGER
	void loadNearChunk() {

		YVec3<int> actualpos = YVec3<int>(actualChunk->_XPos, actualChunk->_YPos, actualChunk->_ZPos);
		// nouveau tri des voisins avec la nouvelle position
		lockNeighbour.lock();
		neighbours.sort([this](const YVec3<int> & a, YVec3<int> & b) { return compareChunk(a, b); });
		lockNeighbour.unlock();
		while (neighbours.size() > 0 && ((*neighbours.begin()) - actualpos).getSize() < ((firstCast)?RADIUS:RADIUSDRAW)) {

			
			vector<thread*> t;
			lockNeighbour.lock();
			while (t.size() < 3 && neighbours.size() > 0 && ((*neighbours.begin()) - actualpos).getSize() < ((firstCast) ? RADIUS : RADIUSDRAW)) {
				lockNeighbour.unlock();
				lockNeighbour.lock();
				YVec3<int> next = *neighbours.begin();
				neighbours.pop_front();
				lockNeighbour.unlock();

				t.push_back(new std::thread([this,next]() {
					addChunk((int)next.X, (int)next.Y, (int)next.Z);
				}));
				lockNeighbour.lock();
				// Tri du tableau
				neighbours.sort([this](const YVec3<int> & a, YVec3<int> & b) { return compareChunk(a, b); });
				while (neighbours.size() > 1000) neighbours.pop_back();
			}
			lockNeighbour.unlock();
			for (int i = 0; i < t.size(); ++i) t[i]->join();
			
			/*
			std::thread t = std::thread([this]() {
				YVec3<int> next = *neighbours.begin();
				neighbours.pop_front();
				addChunk((int)next.X, (int)next.Y, (int)next.Z);
			});
			t.join();
			*/
		}
	}


	// CREATION DUN CHUNK
	void addChunk(int x, int y, int z) {
		addChunk(YVec3<int>(x, y, z));
	}

	void addChunk(YVec3<int> pos) {
		treats.push_back(pos);

		MChunk * chunk = new MChunk(pos.X, pos.Y, pos.Z);
		generateChunk(chunk);

		if(chunk){

			SetNeighboursChunk(chunk);

			lock.lock();
			listChunks.push_back(chunk);
			lock.unlock();

			if (!actualChunk) {
				actualChunk = chunk;
			}
		}

		int x = pos.X, y = pos.Y, z = pos.Z;
		// Ajout des nouvelles frontières
		lockNeighbour.lock();
		if (x - 1 >= 0 && !alreadyTreat(YVec3<int>(x - 1, y, z))) neighbours.push_back(YVec3<int>(x - 1, y, z));
		if (x + 1 < MAT_SIZE && !alreadyTreat(YVec3<int>(x + 1, y, z))) neighbours.push_back(YVec3<int>(x + 1, y, z));
		if (y - 1 >= 0 && !alreadyTreat(YVec3<int>(x, y - 1, z)))	neighbours.push_back(YVec3<int>(x, y - 1, z));
		if (y + 1 < MAT_SIZE && !alreadyTreat(YVec3<int>(x, y + 1, z))) neighbours.push_back(YVec3<int>(x, y + 1, z));
		if (z - 1 >= 0 && !alreadyTreat(YVec3<int>(x, y, z - 1))) neighbours.push_back(YVec3<int>(x, y, z - 1));
		if (z + 1 < MAT_SIZE && !alreadyTreat(YVec3<int>(x, y, z + 1))) neighbours.push_back(YVec3<int>(x, y, z + 1));

		
		//while (neighbours.size() > 1000) neighbours.pop_back();
		lockNeighbour.unlock();
	}

	bool compareChunk(const YVec3<int> & a, YVec3<int> & b) {
		YVec3f A = YVec3f(a.X - actualChunk->_XPos, a.Y - actualChunk->_YPos, a.Z - actualChunk->_ZPos);
		YVec3f B = YVec3f(b.X - actualChunk->_XPos, b.Y - actualChunk->_YPos, b.Z - actualChunk->_ZPos);
		return A.getSize() < B.getSize();
		// (a - YVec3<int>(actualChunk->_XPos, actualChunk->_YPos, actualChunk->_ZPos)).getSize() < 
		// (b - YVec3<int>(actualChunk->_XPos, actualChunk->_YPos, actualChunk->_ZPos)).getSize();
		
	}


	bool alreadyTreat(YVec3<int> pos) {

		for (auto neighbour : neighbours) {
			if (pos == neighbour) return true;
		}

		for (auto treat : treats) {
			if (pos == treat) return true;
		}
		return false;
	}
	void SetNeighboursChunk(MChunk * chunk) {
		lock.lock();
		for (int i = 0; i < listChunks.size(); ++i) {
			// XPREV
			if (listChunks[i]->_XPos == chunk->_XPos + 1 && listChunks[i]->_YPos == chunk->_YPos && listChunks[i]->_ZPos == chunk->_ZPos) {
				listChunks[i]->Voisins[MChunk::Voisin::XPREV] = chunk;
				chunk->MChunk::Voisins[MChunk::Voisin::XNEXT] = listChunks[i];
			}
			// XNEXT
			else if (listChunks[i]->_XPos == chunk->_XPos - 1 && listChunks[i]->_YPos == chunk->_YPos && listChunks[i]->_ZPos == chunk->_ZPos) {
				listChunks[i]->Voisins[MChunk::Voisin::XNEXT] = chunk;
				chunk->Voisins[MChunk::Voisin::XPREV] = listChunks[i];
			}
			// YPREV
			else if (listChunks[i]->_XPos == chunk->_XPos && listChunks[i]->_YPos == chunk->_YPos + 1 && listChunks[i]->_ZPos == chunk->_ZPos) {
				listChunks[i]->Voisins[MChunk::Voisin::YPREV] = chunk;
				chunk->Voisins[MChunk::Voisin::YNEXT] = listChunks[i];
			}
			// YNEXT
			else if (listChunks[i]->_XPos == chunk->_XPos && listChunks[i]->_YPos == chunk->_YPos - 1 && listChunks[i]->_ZPos == chunk->_ZPos) {
				listChunks[i]->Voisins[MChunk::Voisin::YNEXT] = chunk;
				chunk->Voisins[MChunk::Voisin::YPREV] = listChunks[i];
			}
			// ZPREV
			else if (listChunks[i]->_XPos == chunk->_XPos && listChunks[i]->_YPos == chunk->_YPos && listChunks[i]->_ZPos == chunk->_ZPos + 1) {
				listChunks[i]->Voisins[MChunk::Voisin::ZPREV] = chunk;
				chunk->Voisins[MChunk::Voisin::ZNEXT] = listChunks[i];
			}
			// ZNEXT
			else if (listChunks[i]->_XPos == chunk->_XPos && listChunks[i]->_YPos == chunk->_YPos && listChunks[i]->_ZPos == chunk->_ZPos - 1) {
				listChunks[i]->Voisins[MChunk::Voisin::ZNEXT] = chunk;
				chunk->Voisins[MChunk::Voisin::ZPREV] = listChunks[i];
			}

		}
		lock.unlock();
	}



	void generateChunk(MChunk * chunk) {

		chunk->generate();
		chunk->disableHiddenCubes();
		chunk->toVbos();
		toVBOs.push(chunk);
	}
	
	//Boites de collisions plus petites que deux cubes
	MAxis getMinCol(YVec3f pos, YVec3f dir, float width, float height, float & valueColMin, bool oneShot)
	{
		int x = (int)(pos.X / MCube::CUBE_SIZE);
		int y = (int)(pos.Y / MCube::CUBE_SIZE);
		int z = (int)(pos.Z / MCube::CUBE_SIZE);

		int xNext = (int)((pos.X + width / 2.0f) / MCube::CUBE_SIZE);
		int yNext = (int)((pos.Y + width / 2.0f) / MCube::CUBE_SIZE);
		int zNext = (int)((pos.Z + height / 2.0f) / MCube::CUBE_SIZE);

		int xPrev = (int)((pos.X - width / 2.0f) / MCube::CUBE_SIZE);
		int yPrev = (int)((pos.Y - width / 2.0f) / MCube::CUBE_SIZE);
		int zPrev = (int)((pos.Z - height / 2.0f) / MCube::CUBE_SIZE);

		if (x < 0)	x = 0;
		if (y < 0)	y = 0;
		if (z < 0)	z = 0;

		if (xPrev < 0)	xPrev = 0;
		if (yPrev < 0)	yPrev = 0;
		if (zPrev < 0)	zPrev = 0;

		if (xNext < 0)	xNext = 0;
		if (yNext < 0)	yNext = 0;
		if (zNext < 0)	zNext = 0;

		if (x >= MAT_SIZE_CUBES)	x = MAT_SIZE_CUBES - 1;
		if (y >= MAT_SIZE_CUBES)	y = MAT_SIZE_CUBES - 1;
		if (z >= MAT_HEIGHT_CUBES)	z = MAT_HEIGHT_CUBES - 1;

		if (xPrev >= MAT_SIZE_CUBES)	xPrev = MAT_SIZE_CUBES - 1;
		if (yPrev >= MAT_SIZE_CUBES)	yPrev = MAT_SIZE_CUBES - 1;
		if (zPrev >= MAT_HEIGHT_CUBES)	zPrev = MAT_HEIGHT_CUBES - 1;

		if (xNext >= MAT_SIZE_CUBES)	xNext = MAT_SIZE_CUBES - 1;
		if (yNext >= MAT_SIZE_CUBES)	yNext = MAT_SIZE_CUBES - 1;
		if (zNext >= MAT_HEIGHT_CUBES)	zNext = MAT_HEIGHT_CUBES - 1;

		//On fait chaque axe
		MAxis axis = 0x00;
		valueColMin = oneShot ? 0.5f : 10000.0f;
		float seuil = 0.0000001f;
		float prodScalMin = 1.0f;
		if (dir.getSqrSize() > 1)
			dir.normalize();

		//On verif tout les 4 angles de gauche
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev + 1, yPrev, zPrev)->isSolid() ||
				getCube(xPrev + 1, yPrev, zNext)->isSolid() ||
				getCube(xPrev + 1, yNext, zPrev)->isSolid() ||
				getCube(xPrev + 1, yNext, zNext)->isSolid()) || !oneShot)
			{
				float depassement = ((xPrev + 1) * MCube::CUBE_SIZE) - (pos.X - width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_X;
					}
			}
		}

		//float depassementx2 = (xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f);

		//On verif tout les 4 angles de droite
		if (getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xNext - 1, yPrev, zPrev)->isSolid() ||
				getCube(xNext - 1, yPrev, zNext)->isSolid() ||
				getCube(xNext - 1, yNext, zPrev)->isSolid() ||
				getCube(xNext - 1, yNext, zNext)->isSolid()) || !oneShot)
			{
				float depassement = (xNext * MCube::CUBE_SIZE) - (pos.X + width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_X;
					}
			}
		}

		//float depassementy1 = (yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f);

		//On verif tout les 4 angles de devant
		if (getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yNext - 1, zPrev)->isSolid() ||
				getCube(xPrev, yNext - 1, zNext)->isSolid() ||
				getCube(xNext, yNext - 1, zPrev)->isSolid() ||
				getCube(xNext, yNext - 1, zNext)->isSolid()) || !oneShot)
			{
				float depassement = (yNext * MCube::CUBE_SIZE) - (pos.Y + width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Y;
					}
			}
		}

		//float depassementy2 = ((yPrev + 1) * NYCube::CUBE_SIZE) - (pos.Y - width / 2.0f);

		//On verif tout les 4 angles de derriere
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev + 1, zPrev)->isSolid() ||
				getCube(xPrev, yPrev + 1, zNext)->isSolid() ||
				getCube(xNext, yPrev + 1, zPrev)->isSolid() ||
				getCube(xNext, yPrev + 1, zNext)->isSolid()) || !oneShot)
			{
				float depassement = ((yPrev + 1) * MCube::CUBE_SIZE) - (pos.Y - width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Y;
					}
			}
		}

		//On verif tout les 4 angles du haut
		if (getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zNext - 1)->isSolid() ||
				getCube(xPrev, yNext, zNext - 1)->isSolid() ||
				getCube(xNext, yPrev, zNext - 1)->isSolid() ||
				getCube(xNext, yNext, zNext - 1)->isSolid()) || !oneShot)
			{
				float depassement = (zNext * MCube::CUBE_SIZE) - (pos.Z + height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Z;
					}
			}
		}

		//On verif tout les 4 angles du bas
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zPrev + 1)->isSolid() ||
				getCube(xPrev, yNext, zPrev + 1)->isSolid() ||
				getCube(xNext, yPrev, zPrev + 1)->isSolid() ||
				getCube(xNext, yNext, zPrev + 1)->isSolid()) || !oneShot)
			{
				float depassement = ((zPrev + 1) * MCube::CUBE_SIZE) - (pos.Z - height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Z;
					}
			}
		}

		return axis;
	}
		

	void render_world_vbo(int shader, YTexFile * texture, bool debug, bool doTransparent)
	{
		
		//add_world_to_vbo();
		YEngine * engine = YEngine::getInstance();
		YVec3f camPos = engine->Renderer->Camera->Position;
		YVec3f camDir = engine->Renderer->Camera->Direction;
		glDisable(GL_BLEND);
		//Dessiner les chunks opaques
		lock.lock();
		for (int i = 0; i < listChunks.size(); ++i) {
		
			MChunk * chunk = listChunks[i];
			
			//chunk->toVbos();
			if (chunk->draw && !chunk->SetHide(camPos, camDir)) {
				glPushMatrix();
				glTranslatef(chunk->_XPos * MCubes::CHUNK_SIZE, chunk->_YPos *MCubes::CHUNK_SIZE, chunk->_ZPos * MCubes::CHUNK_SIZE);
				glUseProgram(shader);
				YEngine::getInstance()->Renderer->updateMatricesFromOgl(); //Calcule toute les matrices à partir des deux matrices OGL
				YEngine::getInstance()->Renderer->sendMatricesToShader(shader); //Envoie les matrices au shader
				texture->setAsShaderInput(shader);
				chunk->VboOpaque->render();
				glPopMatrix();
			}
		}
		glEnable(GL_BLEND);
		//Dessiner les chunks transparents
		for (int i = 0; i < listChunks.size(); ++i) {

			MChunk * chunk = listChunks[i];
			//chunk->toVbos();
			if (chunk->draw && !chunk->SetHide(camPos, camDir)) {
				glPushMatrix();
				glTranslatef(chunk->_XPos * MCubes::CHUNK_SIZE, chunk->_YPos *MCubes::CHUNK_SIZE, chunk->_ZPos * MCubes::CHUNK_SIZE);
				glUseProgram(shader);
				YEngine::getInstance()->Renderer->updateMatricesFromOgl(); //Calcule toute les matrices à partir des deux matrices OGL
				YEngine::getInstance()->Renderer->sendMatricesToShader(shader); //Envoie les matrices au shader
				texture->setAsShaderInput(shader);
				chunk->VboTransparent->render();
				glPopMatrix();
			}
		}
		lock.unlock();
	}

	/**
	* Attention ce code n'est pas optimal, il est compréhensible. Il existe de nombreuses
	* versions optimisées de ce calcul.
	*/
	inline bool intersecDroitePlan(const YVec3f & debSegment, const  YVec3f & finSegment,
		const YVec3f & p1Plan, const YVec3f & p2Plan, const YVec3f & p3Plan,
		YVec3f & inter)
	{
		
		return true;
	}

	/**
	* Attention ce code n'est pas optimal, il est compréhensible. Il existe de nombreuses
	* versions optimisées de ce calcul. Il faut donner les points dans l'ordre (CW ou CCW)
	*/
	inline bool intersecDroiteCubeFace(const YVec3f & debSegment, const YVec3f & finSegment,
		const YVec3f & p1, const YVec3f & p2, const YVec3f & p3, const  YVec3f & p4,
		YVec3f & inter)
	{
		
		return false;
	}

	bool getRayCollision(const YVec3f & debSegment, const YVec3f & finSegment,
		YVec3f & inter,
		int &xCube, int&yCube, int&zCube)
	{
		
		return false;
	}

	/**
	* De meme cette fonction peut être grandement opitimisée, on a priviligié la clarté
	*/
	bool getRayCollisionWithCube(const YVec3f & debSegment, const YVec3f & finSegment,
		int x, int y, int z,
		YVec3f & inter)
	{

		return true;
	}
};



#endif