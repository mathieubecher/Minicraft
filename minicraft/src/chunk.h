#pragma once

#include "engine/render/renderer.h"
#include "engine/render/vbo.h"
#include "cube.h"
#include "atlasMap.h"
#include "MCubes.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>    
#include <mutex>

/**
  * On utilise des chunks pour que si on modifie juste un cube, on ait pas
  * besoin de recharger toute la carte dans le buffer, mais juste le chunk en question
  */
class YFile;

class MChunk
{
public:
	bool resetSeed = false; 

	void WriteChunk() {
		//auto start = std::chrono::system_clock::now();
		MCubes * cubes = dynamic_cast<MCubes*>(_Cubes);

		uint8* buffer = new uint8[MCubes::CHUNK_SIZE * MCubes::CHUNK_SIZE * MCubes::CHUNK_HEIGHT*4];
		
		uint8 cubeType = cubes->_Cubes[0][0][0]._Code;
		//*
		uint16 nbOccur = 0;
		bool endnull;
		bool basenull;
		int posInFile = -1;
		for (int z = 0; z < MCubes::CHUNK_HEIGHT; ++z)
			for (int y = 0; y < MCubes::CHUNK_SIZE; ++y) {
				for (int x = 0; x < MCubes::CHUNK_SIZE; ++x) {
					if ((cubes->_Cubes[x][y][z]._Code != cubeType || nbOccur == 0xFFFF) || (z + 1 == MCubes::CHUNK_HEIGHT && y + 1 == MCubes::CHUNK_SIZE && x + 1 == MCubes::CHUNK_SIZE)) {
						if (z + 1 == MCubes::CHUNK_HEIGHT && y + 1 == MCubes::CHUNK_SIZE && x + 1 == MCubes::CHUNK_SIZE) ++nbOccur;
						endnull = (nbOccur & 0xFF00) == 0x0000;
						basenull = (nbOccur & 0x00FF) == 0x0000;
						buffer[++posInFile] = (MCube::MCubeType)cubeType;
						buffer[++posInFile] = 0x80 | basenull | (endnull << 1);
						++posInFile;
						if (!endnull) buffer[posInFile] = nbOccur >> 8;
						++posInFile;
						if (!basenull) buffer[posInFile] = nbOccur;

						nbOccur = 0;
						cubeType = (MCube::MCubeType)cubes->_Cubes[x][y][z]._Code;
					}
					++nbOccur;
				}
			}

		buffer[++posInFile] = 0x00;

		string path = ToPath(_XPos, _YPos, _ZPos);

		ofstream myfile(path, std::ofstream::binary);
		myfile << buffer;

		buffer[posInFile] = 0x01;
		delete[] buffer;
		myfile.close();

		//auto end = std::chrono::system_clock::now();
		//std::chrono::duration<double> time = end - start;
		//std::cout << time.count() * 1000 << endl;
	}

	void ReadChunk() {
		if (Exist(_XPos, _YPos, _ZPos)) {
			MCubes * cubes = dynamic_cast<MCubes*>(_Cubes);
			//auto start = std::chrono::system_clock::now();

			ifstream myfile(ToPath(_XPos, _YPos, _ZPos), std::ifstream::binary);
			std::filebuf* pbuf = myfile.rdbuf();
			int size = pbuf->pubseekoff(0, myfile.end, myfile.in);
			pbuf->pubseekpos(0, myfile.in);
			uint8* buffer = new uint8[size];
			pbuf->sgetn((char*)buffer, size);
			myfile.close();
			//auto fileopen = std::chrono::system_clock::now();

			int posInFile = -1;
			uint8 cubeType;
			uint16 nbOccur = 0;
			uint8 occurinfo;

			for (int z = 0; z < MCubes::CHUNK_HEIGHT; ++z)
				for (int y = 0; y < MCubes::CHUNK_SIZE; ++y) {
					for (int x = 0; x < MCubes::CHUNK_SIZE; ++x) {
						if (nbOccur == 0) {
							cubeType = (MCube::MCubeType)buffer[++posInFile];
							occurinfo = buffer[++posInFile];
							++posInFile;
							if ((occurinfo & 2) == 0) nbOccur = buffer[posInFile] << 8;
							++posInFile;
							if ((occurinfo & 1) == 0) nbOccur |= buffer[posInFile];
						}
						nbOccur--;
						cubes->_Cubes[x][y][z]._Code= cubeType;
					}
				}
			delete[] buffer;
			//auto end = std::chrono::system_clock::now();
			//std::chrono::duration<double> time = end - start;
			//std::cout << time.count() * 1000 << endl;
		}
	}
	static bool Exist(int x, int y, int z) {
		ifstream myfile;
		myfile.open(ToPath(x, y, z));
		bool exist = myfile.is_open();
		myfile.close();
		return exist;
	}
	static string ToPath(int x, int y, int z) {
		return "world/chunk/chunk" + toString(x) + "_" + toString(y) + "_" + toString(z)+ ".mnc";
	}

	enum MCubeType
	{
		TOP = 1,
		BOTTOM,
		SIDE
	};
	
	MCubesClear *_Cubes;

	YVbo * VboOpaque = NULL;
	YVbo * VboTransparent = NULL;

	YVbo * NewVboOpaque = NULL;
	YVbo * NewVboTransparent = NULL;

	MChunk * Voisins[6];
	mutex vboLock;

	enum Voisin {
		XPREV = 0,
		XNEXT = 1,
		YPREV = 2,
		YNEXT = 3,
		ZPREV = 4,
		ZNEXT = 5
	};

	int _XPos, _YPos, _ZPos; ///< Position du chunk dans le monde

	bool hide = true;
	bool draw = false;
	bool physic = false;
	bool vbo = false;
	bool reload = true;

	MChunk(int x, int y, int z) 
	{
		_Cubes = new MCubes();
		memset(Voisins, 0x00, sizeof(void*) * 6);
		_XPos = x;
		_YPos = y;
		_ZPos = z;

	}
	~MChunk() {
		//cout << "suppression du chunk [" << _XPos << ";" << _YPos << ";" << _ZPos << "]"<<endl;
		delete _Cubes;
		if (Voisins[XPREV] != NULL) Voisins[XPREV]->Voisins[XNEXT] = NULL;
		if (Voisins[XNEXT] != NULL) Voisins[XNEXT]->Voisins[XPREV] = NULL;
		if (Voisins[YPREV] != NULL) Voisins[YPREV]->Voisins[YNEXT] = NULL;
		if (Voisins[YNEXT] != NULL) Voisins[YNEXT]->Voisins[YPREV] = NULL;
		if (Voisins[ZPREV] != NULL) Voisins[ZPREV]->Voisins[ZNEXT] = NULL;
		if (Voisins[ZNEXT] != NULL) Voisins[ZNEXT]->Voisins[ZPREV] = NULL;
	}
	bool SetHide(YVec3f pos, YVec3f dir) {
		hide = Hide(pos, dir);
		return hide;
	}
	bool Hide(YVec3f pos, YVec3f dir) {
		YVec3f chunkPosBase = YVec3f(_XPos * MCubes::CHUNK_SIZE, _YPos * MCubes::CHUNK_SIZE, _ZPos * MCubes::CHUNK_HEIGHT);
		float maxAngle = 0;
		YVec3f chunkDir = (chunkPosBase + YVec3f(0,0,MCubes::CHUNK_HEIGHT) - pos).normalize();
		float angle = chunkDir.dot(dir) / (sqrt(pow(chunkDir.X, 2) + pow(chunkDir.Y, 2) + pow(chunkDir.Z, 2))*sqrt(pow(dir.X, 2) + pow(dir.Y, 2) + pow(dir.Z, 2)));
		if(angle > maxAngle) return false;

		chunkDir = (chunkPosBase + YVec3f(MCubes::CHUNK_SIZE, 0, MCubes::CHUNK_HEIGHT) - pos).normalize();
		angle = chunkDir.dot(dir) / (sqrt(pow(chunkDir.X, 2) + pow(chunkDir.Y, 2) + pow(chunkDir.Z, 2))*sqrt(pow(dir.X, 2) + pow(dir.Y, 2) + pow(dir.Z, 2)));
		if (angle > maxAngle) return false;

		chunkDir = (chunkPosBase + YVec3f(MCubes::CHUNK_SIZE, MCubes::CHUNK_SIZE, MCubes::CHUNK_HEIGHT) - pos).normalize();
		angle = chunkDir.dot(dir) / (sqrt(pow(chunkDir.X, 2) + pow(chunkDir.Y, 2) + pow(chunkDir.Z, 2))*sqrt(pow(dir.X, 2) + pow(dir.Y, 2) + pow(dir.Z, 2)));
		if (angle > maxAngle) return false;

		chunkDir = (chunkPosBase + YVec3f(0, MCubes::CHUNK_SIZE, MCubes::CHUNK_HEIGHT) - pos).normalize();
		angle = chunkDir.dot(dir) / (sqrt(pow(chunkDir.X, 2) + pow(chunkDir.Y, 2) + pow(chunkDir.Z, 2))*sqrt(pow(dir.X, 2) + pow(dir.Y, 2) + pow(dir.Z, 2)));
		if (angle > maxAngle) return false;

		chunkDir = (chunkPosBase + YVec3f(0, 0, 0) - pos).normalize();
		angle = chunkDir.dot(dir) / (sqrt(pow(chunkDir.X, 2) + pow(chunkDir.Y, 2) + pow(chunkDir.Z, 2))*sqrt(pow(dir.X, 2) + pow(dir.Y, 2) + pow(dir.Z, 2)));
		if (angle > maxAngle) return false;

		chunkDir = (chunkPosBase + YVec3f(MCubes::CHUNK_SIZE, 0, 0) - pos).normalize();
		angle = chunkDir.dot(dir) / (sqrt(pow(chunkDir.X, 2) + pow(chunkDir.Y, 2) + pow(chunkDir.Z, 2))*sqrt(pow(dir.X, 2) + pow(dir.Y, 2) + pow(dir.Z, 2)));
		if (angle > maxAngle) return false;

		chunkDir = (chunkPosBase + YVec3f(MCubes::CHUNK_SIZE, MCubes::CHUNK_SIZE, 0) - pos).normalize();
		angle = chunkDir.dot(dir) / (sqrt(pow(chunkDir.X, 2) + pow(chunkDir.Y, 2) + pow(chunkDir.Z, 2))*sqrt(pow(dir.X, 2) + pow(dir.Y, 2) + pow(dir.Z, 2)));
		if (angle > maxAngle) return false;

		chunkDir = (chunkPosBase + YVec3f(0, MCubes::CHUNK_SIZE, 0) - pos).normalize();
		angle = chunkDir.dot(dir) / (sqrt(pow(chunkDir.X, 2) + pow(chunkDir.Y, 2) + pow(chunkDir.Z, 2))*sqrt(pow(dir.X, 2) + pow(dir.Y, 2) + pow(dir.Z, 2)));
		if (angle > maxAngle) return false;

		
		return true;
	}

	void deleteCubes() {
		physic = false;
		//cout << "delete : " << sizeof(_Cubes) << endl << endl << endl << endl;
		delete _Cubes;
		_Cubes = new MCubesClear();

		//cout<<"Suppression Physic : " << _XPos << ", "<<_YPos<<", "<<_ZPos << endl;
	}
	void reloadCubes() {
		physic = true;
		delete _Cubes;
		_Cubes = new MCubes();
		//cout << "Recréation Physic : " << _XPos << ", " << _YPos << ", " << _ZPos << endl;

	}
	void generate() {

		if (Exist(_XPos, _YPos, _ZPos) && !resetSeed) ReadChunk();
		else {
			reset();
			_Cubes->generate(_XPos, _YPos, _ZPos);
			if (!resetSeed) WriteChunk();
		}
		physic = true;
	}
	/*
	Creation des VBO
	*/
	int * nbVertOpaque = new int();
	int * nbVertTransp = new int();
	bool updateVert = true;

	void toNewVbos(void) {
		//Compter les sommets
		if (updateVert) {
			foreachVisibleTriangle(true, nbVertOpaque, nbVertTransp, NewVboOpaque, NewVboTransparent);
			updateVert = false;
		}
		NewVboOpaque = new YVbo(4, (*nbVertOpaque), YVbo::PACK_BY_ELEMENT_TYPE);
		NewVboTransparent = new YVbo(4, (*nbVertOpaque), YVbo::PACK_BY_ELEMENT_TYPE);
		//Créer les VBO
		NewVboOpaque = new YVbo(4, (*nbVertOpaque), YVbo::PACK_BY_ELEMENT_TYPE);
		NewVboOpaque->setElementDescription(0, YVbo::Element(3)); //Sommet
		NewVboOpaque->setElementDescription(1, YVbo::Element(3)); //Normale
		NewVboOpaque->setElementDescription(2, YVbo::Element(2)); //UV
		NewVboOpaque->setElementDescription(3, YVbo::Element(1)); //Type
		NewVboOpaque->createVboCpu();

		NewVboTransparent = new YVbo(4, (*nbVertTransp), YVbo::PACK_BY_ELEMENT_TYPE);
		NewVboTransparent->setElementDescription(0, YVbo::Element(3)); //Sommet
		NewVboTransparent->setElementDescription(1, YVbo::Element(3)); //Normale
		NewVboTransparent->setElementDescription(2, YVbo::Element(2)); //UV
		NewVboTransparent->setElementDescription(3, YVbo::Element(1)); //Type
		NewVboTransparent->createVboCpu();
		//Remplir les VBO
		foreachVisibleTriangle(false, nbVertOpaque, nbVertTransp, NewVboOpaque, NewVboTransparent);
		vbo = true;
	}
	//On met le chunk ddans son VBO
	void toVbos(void)
	{
		toNewVbos();
		/*
		SAFEDELETE(VboOpaque);
		SAFEDELETE(VboTransparent);
		
		//Compter les sommets
		if (updateVert) {
			foreachVisibleTriangle(true, nbVertOpaque, nbVertTransp, VboOpaque, VboTransparent);
			updateVert = false;
		}
			
		//Créer les VBO
		VboOpaque = new YVbo(4, (*nbVertOpaque), YVbo::PACK_BY_ELEMENT_TYPE);
		VboOpaque->setElementDescription(0, YVbo::Element(3)); //Sommet
		VboOpaque->setElementDescription(1, YVbo::Element(3)); //Normale
		VboOpaque->setElementDescription(2, YVbo::Element(2)); //UV
		VboOpaque->setElementDescription(3, YVbo::Element(1)); //Type
		VboOpaque->createVboCpu();

		VboTransparent = new YVbo(4, (*nbVertTransp), YVbo::PACK_BY_ELEMENT_TYPE);
		VboTransparent->setElementDescription(0, YVbo::Element(3)); //Sommet
		VboTransparent->setElementDescription(1, YVbo::Element(3)); //Normale
		VboTransparent->setElementDescription(2, YVbo::Element(2)); //UV
		VboTransparent->setElementDescription(3, YVbo::Element(1)); //Type
		VboTransparent->createVboCpu();
		//Remplir les VBO
		foreachVisibleTriangle(false, nbVertOpaque, nbVertTransp, VboOpaque, VboTransparent);
		vbo = true;
		*/
	}

	void CreateVboGpu() {
		SAFEDELETE(VboOpaque);
		SAFEDELETE(VboTransparent);

		VboOpaque = NewVboOpaque;
		VboTransparent = NewVboTransparent;
		// On envoie le contenu au GPU
		VboOpaque->createVboGpu();
		// On clean le contenu du CPU
		VboOpaque->deleteVboCpu();
		// On envoie le contenu au GPU
		VboTransparent->createVboGpu();
		// On clean le contenu du CPU
		VboTransparent->deleteVboCpu();
		NewVboOpaque = NULL;
		NewVboTransparent = NULL;
	}

	//Ajoute un quad du cube. Attention CCW
	int addQuadToVbo(YVbo * vbo, int iVertice, YVec3f & a, YVec3f & b, YVec3f & c, YVec3f & d, float type, float dir) {
		int iVerticeQuad = 0;
		AtlasMap * instance = AtlasMap::GetInstance();
		AtlasId id = instance->atlas[(int)type];

		int idu = id.sideU, idv = id.sideV;

		if (dir == MCubeType::TOP) {
			idu = id.topU;
			idv = id.topV;
		}
		else if (dir == MCubeType::BOTTOM) {
			idu = id.bottomU;
			idv = id.bottomV;
		}
		YVec3f normal = (b-a).cross(c-a).normalize();
		vbo->setElementValue(0, iVertice + iVerticeQuad, a.X, a.Y, a.Z);
		vbo->setElementValue(1, iVertice + iVerticeQuad, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice + iVerticeQuad, (idu + 0) / 16.0f, (idv + 1) / 16.0f);
		vbo->setElementValue(3, iVertice + iVerticeQuad, type);
		++iVerticeQuad;
		vbo->setElementValue(0, iVertice + iVerticeQuad, b.X, b.Y, b.Z);
		vbo->setElementValue(1, iVertice + iVerticeQuad, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice + iVerticeQuad, (idu + 1) / 16.0f, (idv + 1) / 16.0f);
		vbo->setElementValue(3, iVertice + iVerticeQuad, type);
		++iVerticeQuad;
		vbo->setElementValue(0, iVertice + iVerticeQuad, c.X, c.Y, c.Z);
		vbo->setElementValue(1, iVertice + iVerticeQuad, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice + iVerticeQuad, (idu + 1) / 16.0f, (idv + 0) / 16.0f);
		vbo->setElementValue(3, iVertice + iVerticeQuad, type);
		++iVerticeQuad;


		vbo->setElementValue(0, iVertice + iVerticeQuad, a.X, a.Y, a.Z);
		vbo->setElementValue(1, iVertice + iVerticeQuad, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice + iVerticeQuad, (idu + 0) / 16.0f, (idv + 1) / 16.0f);
		vbo->setElementValue(3, iVertice + iVerticeQuad, type);
		++iVerticeQuad;
		vbo->setElementValue(0, iVertice + iVerticeQuad, c.X, c.Y, c.Z);
		vbo->setElementValue(1, iVertice + iVerticeQuad, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice + iVerticeQuad, (idu + 1) / 16.0f, (idv + 0) / 16.0f);
		vbo->setElementValue(3, iVertice + iVerticeQuad, type);
		++iVerticeQuad;
		vbo->setElementValue(0, iVertice + iVerticeQuad, d.X, d.Y, d.Z);
		vbo->setElementValue(1, iVertice + iVerticeQuad, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice + iVerticeQuad, (idu + 0) / 16.0f, (idv + 0) / 16.0f);
		vbo->setElementValue(3, iVertice + iVerticeQuad, type);
		++iVerticeQuad;

		return iVerticeQuad;
	}

	//Permet de compter les triangles ou des les ajouter aux VBO
	void foreachVisibleTriangle(bool countOnly, int * nbVertOpaque, int * nbVertTransp, YVbo * VboOpaque, YVbo * VboTrasparent) {
		//auto start = std::chrono::system_clock::now();
		int iVerticeOpaque = 0;
		int iVerticeTransp = 0;
		for (int x = 0; x < MCubes::CHUNK_SIZE; ++x)
			for (int y = 0; y < MCubes::CHUNK_SIZE; ++y)
				for (int z = 0; z < MCubes::CHUNK_HEIGHT; ++z) {
					uint8 cube = _Cubes->get(x, y,z)->_Code & ~MCube::CUBE_DRAW_BIT;
					bool draw = _Cubes->get(x, y, z)->_Code & MCube::CUBE_DRAW_BIT ? true : false;
					
					if (draw ) {
						bool opaque = cube != MCube::CUBE_AIR && cube != MCube::CUBE_EAU && cube != MCube::CUBE_VERRE && cube != MCube::CUBE_BRANCHES;
						bool transparent = (cube == MCube::CUBE_EAU || cube == MCube::CUBE_VERRE);
						if (countOnly) {
							
							int _NbVert = 0;
							if (z - 1 < 0 || (opaque &&  _Cubes->get(x,y,z - 1)->isTransparent() || !opaque && !_Cubes->get(x,y,z - 1)->isPickable())) _NbVert += 6;
							if (z + 1 >= MCubes::CHUNK_HEIGHT || (opaque &&  _Cubes->get(x,y,z + 1)->isTransparent() || !opaque && !_Cubes->get(x,y,z + 1)->isPickable())) _NbVert += 6;
							if (y - 1 < 0 || (opaque &&  _Cubes->get(x,y - 1,z)->isTransparent() || !opaque && !_Cubes->get(x,y - 1,z)->isPickable())) _NbVert += 6;
							if (y + 1 >= MCubes::CHUNK_SIZE || (opaque &&  _Cubes->get(x,y + 1,z)->isTransparent() || !opaque && !_Cubes->get(x,y + 1,z)->isPickable())) _NbVert += 6;
							if (x - 1 < 0 || (opaque &&  _Cubes->get(x - 1,y,z)->isTransparent() || !opaque && !_Cubes->get(x - 1,y,z)->isPickable())) _NbVert += 6;
							if (x + 1 >= MCubes::CHUNK_SIZE || (opaque &&  _Cubes->get(x + 1,y,z)->isTransparent() || !opaque && !_Cubes->get(x + 1,y,z)->isPickable())) _NbVert += 6;

							if (transparent)
								*nbVertTransp += _NbVert;
							else
								*nbVertOpaque += _NbVert;

						}
						else {
							int iVertice = iVerticeOpaque;
							if (transparent)iVertice = iVerticeTransp;
							YVec3f size(1, 1, 1);
							if (transparent && (z>=MCubes::CHUNK_HEIGHT || !_Cubes->get(x,y,z+1)->isPickable())) size.Z = 0.9f;
							YVec3f a = YVec3f(x, y, z);
							YVec3f b = YVec3f(x, y + size.Y, z);
							YVec3f c = YVec3f(x + size.X, y + size.Y, z);
							YVec3f d = YVec3f(x + size.X, y, z);
							YVec3f e = YVec3f(x, y + size.Y, z + size.Z);
							YVec3f f = YVec3f(x, y, z + size.Z);
							YVec3f g = YVec3f(x + size.X, y, z + size.Z);
							YVec3f h = YVec3f(x + size.X, y + size.Y, z + size.Z);

							// XY
							if(z - 1 < 0 || (opaque &&  _Cubes->get(x, y, z - 1)->isTransparent() || !opaque && !_Cubes->get(x,y,z - 1)->isPickable())){
								iVertice += addQuadToVbo((!transparent) ? VboOpaque : VboTrasparent, iVertice, a, b, c, d, cube, MCubeType::BOTTOM);
							}
							if (z + 1 >= MCubes::CHUNK_HEIGHT || (opaque &&  _Cubes->get(x,y,z + 1)->isTransparent() || !opaque && !_Cubes->get(x,y,z + 1)->isPickable())) {
								iVertice += addQuadToVbo((!transparent) ? VboOpaque : VboTrasparent, iVertice, e, f, g, h, cube, MCubeType::TOP);
							}

							// XZ
							if (y - 1 < 0 || (opaque &&  _Cubes->get(x,y - 1,z)->isTransparent() || !opaque &&  !_Cubes->get(x,y - 1,z)->isPickable())) {
								iVertice += addQuadToVbo((!transparent) ? VboOpaque : VboTrasparent, iVertice, a, d, g, f, cube, MCubeType::SIDE);
							}
							if (y + 1 >= MCubes::CHUNK_SIZE || (opaque &&  _Cubes->get(x,y + 1,z)->isTransparent() || !opaque && !_Cubes->get(x,y + 1,z)->isPickable())) {
								iVertice += addQuadToVbo((!transparent) ? VboOpaque : VboTrasparent, iVertice, c, b, e, h, cube, MCubeType::SIDE);
							}
							// YZ
							if (x - 1 < 0 || (opaque &&  _Cubes->get(x - 1,y,z)->isTransparent() || !opaque && !_Cubes->get(x - 1,y,z)->isPickable())) {								
								iVertice += addQuadToVbo((!transparent) ? VboOpaque : VboTrasparent, iVertice, b, a, f, e, cube, MCubeType::SIDE);
							}
							if (x + 1 >= MCubes::CHUNK_SIZE || (opaque &&  _Cubes->get(x + 1,y,z)->isTransparent() || !opaque && !_Cubes->get(x + 1,y,z)->isPickable())) {
								iVertice += addQuadToVbo((!transparent) ? VboOpaque : VboTrasparent, iVertice, d, c, h, g, cube, MCubeType::SIDE);
							}
							if (transparent)iVerticeTransp = iVertice;
							else iVerticeOpaque = iVertice;
						}
					}
				}
		//auto end = std::chrono::system_clock::now();
		//std::chrono::duration<double> time = end - start;
		//std::cout << time.count() * 1000 << endl;
	}

	/*
	Gestion du chunk
	*/

	void reset(void)
	{
		for (int x = 0; x < MCubes::CHUNK_SIZE; x++)
			for (int y = 0; y < MCubes::CHUNK_SIZE; y++)
				for (int z = 0; z < MCubes::CHUNK_HEIGHT; z++)
				{
					_Cubes->get(x,y,z)->setDraw(false);
					_Cubes->get(x, y, z)->setType(MCube::CUBE_AIR);
				}
	}

	void setVoisins(MChunk * xprev, MChunk * xnext, MChunk * yprev, MChunk * ynext, MChunk * zprev, MChunk * znext)
	{
		Voisins[0] = xprev;
		Voisins[1] = xnext;
		Voisins[2] = yprev;
		Voisins[3] = ynext;
		Voisins[4] = zprev;
		Voisins[5] = znext;
	}

	void get_surrounding_cubes(int x, int y, int z, MCube ** cubeXPrev, MCube ** cubeXNext,
		MCube ** cubeYPrev, MCube ** cubeYNext,
		MCube ** cubeZPrev, MCube ** cubeZNext)
	{

		*cubeXPrev = NULL;
		*cubeXNext = NULL;
		*cubeYPrev = NULL;
		*cubeYNext = NULL;
		*cubeZPrev = NULL;
		*cubeZNext = NULL;

		if (x == 0 && Voisins[0] != NULL)
			*cubeXPrev = (Voisins[0]->_Cubes->get(MCubes::CHUNK_SIZE - 1,y,z));
		else if (x > 0)
			*cubeXPrev = (_Cubes->get(x - 1,y,z));

		if (x == MCubes::CHUNK_SIZE - 1 && Voisins[1] != NULL)
			*cubeXNext = (Voisins[1]->_Cubes->get(0,y,z));
		else if (x < MCubes::CHUNK_SIZE - 1)
			*cubeXNext = (_Cubes->get(x + 1,y,z));

		if (y == 0 && Voisins[2] != NULL)
			*cubeYPrev = (Voisins[2]->_Cubes->get(x,MCubes::CHUNK_SIZE - 1,z));
		else if (y > 0)
			*cubeYPrev = (_Cubes->get(x,y - 1,z));

		if (y == MCubes::CHUNK_SIZE - 1 && Voisins[3] != NULL)
			*cubeYNext = (Voisins[3]->_Cubes->get(x,0,z));
		else if (y < MCubes::CHUNK_SIZE - 1)
			*cubeYNext = (_Cubes->get(x,y + 1,z));

		if (z == 0 && Voisins[4] != NULL)
			*cubeZPrev = (Voisins[4]->_Cubes->get(x,y,MCubes::CHUNK_HEIGHT - 1));
		else if (z > 0)
			*cubeZPrev = (_Cubes->get(x,y,z - 1));

		if (z == MCubes::CHUNK_HEIGHT - 1 && Voisins[5] != NULL)
			*cubeZNext = (Voisins[5]->_Cubes->get(x,y,0));
		else if (z < MCubes::CHUNK_HEIGHT - 1)
			*cubeZNext = (_Cubes->get(x,y,z + 1));
	}

	void render(bool transparent)
	{
		
		if (transparent)
			VboTransparent->render();
		else
			VboOpaque->render();
	}

	/**
	  * On verifie si le cube peut être vu
	  */
	bool test_hidden(int x, int y, int z)
	{
		MCube * cubeXPrev = NULL;
		MCube * cubeXNext = NULL;
		MCube * cubeYPrev = NULL;
		MCube * cubeYNext = NULL;
		MCube * cubeZPrev = NULL;
		MCube * cubeZNext = NULL;

		/*if (x == 0 && Voisins[0] != NULL)
			cubeXPrev = (Voisins[0]->_Cubes->get(MCubes::CHUNK_SIZE - 1,y,z));
		else*/ if (x > 0)
			cubeXPrev = (_Cubes->get(x - 1,y,z));

		/*if (x == MCubes::CHUNK_SIZE - 1 && Voisins[1] != NULL)
			cubeXNext = (Voisins[1]->_Cubes->get(0,y,z));
		else */ if (x < MCubes::CHUNK_SIZE - 1)
			cubeXNext = (_Cubes->get(x + 1,y,z));

		/*if (y == 0 && Voisins[2] != NULL)
			cubeYPrev = (Voisins[2]->_Cubes->get(x,MCubes::CHUNK_SIZE - 1,z));
		else */ if (y > 0)
			cubeYPrev = (_Cubes->get(x,y - 1,z));

		/*if (y == MCubes::CHUNK_SIZE - 1 && Voisins[3] != NULL)
			cubeYNext = (Voisins[3]->_Cubes->get(x,0,z));
		else */ if (y < MCubes::CHUNK_SIZE - 1)
			cubeYNext = (_Cubes->get(x,y + 1,z));

		/*if (z == 0 && Voisins[4] != NULL)
			cubeZPrev = (Voisins[4]->_Cubes->get(x,y,MCubes::CHUNK_HEIGHT - 1));
		else */ if (z > 0)
			cubeZPrev = (_Cubes->get(x,y,z - 1));

		/*if (z == MCubes::CHUNK_HEIGHT - 1 && Voisins[5] != NULL)
			cubeZNext = (Voisins[5]->_Cubes->get(x,y,0));
		else */ if (z < MCubes::CHUNK_HEIGHT - 1)
			cubeZNext = (_Cubes->get(x,y,z + 1));

		if (cubeXPrev == NULL || cubeXNext == NULL ||
			cubeYPrev == NULL || cubeYNext == NULL ||
			cubeZPrev == NULL || cubeZNext == NULL)
			return false;

		if (cubeXPrev->isOpaque() == true && //droite
			cubeXNext->isOpaque() == true && //gauche
			cubeYPrev->isOpaque() == true && //haut
			cubeYNext->isOpaque() == true && //bas
			cubeZPrev->isOpaque() == true && //devant
			cubeZNext->isOpaque() == true)  //derriere
			return true;
		return false;
	}

	void disableHiddenCubes(void)
	{
		for (int x = 0; x < MCubes::CHUNK_SIZE; x++)
			for (int y = 0; y < MCubes::CHUNK_SIZE; y++)
				for (int z = 0; z < MCubes::CHUNK_HEIGHT; z++)
				{
					
					_Cubes->get(x, y, z)->setDraw(true);
					if (!_Cubes->get(x, y, z)->isPickable() || test_hidden(x, y, z))
						_Cubes->get(x, y, z)->setDraw(false);
				}
	}


};
