#pragma once

#include "cube.h"

class AtlasId {
public:
	int topU;
	int topV;
	int sideU;
	int sideV;
	int bottomU;
	int bottomV;


	AtlasId() {
		topU = 0;
		topV = 0;
		sideU = 0;
		sideV = 0;
		bottomU = 0;
		bottomV = 0;
	}

	AtlasId(int topu,int topv,int sideu,int sidev,int bottomu,int bottomv) {


		topU = topu;
		topV = topv;
		sideU = sideu;
		sideV = sidev;
		bottomU = bottomu;
		bottomV = bottomv;
	}
};
class AtlasMap
{
private :
	inline static AtlasMap * instance;
public:
	
	static AtlasMap * GetInstance(void) {
		if (instance == NULL)
			instance = new AtlasMap();
		return instance;
	}

	YTexFile * terrain;
	AtlasId atlas[126];
	AtlasMap()
	{
		YTexManager * texManager = YTexManager::getInstance();
		terrain = texManager->loadTexture("texture/terrain.png");
		texManager->loadTextureToOgl(*terrain);
		atlas[MCube::MCubeType::CUBE_HERBE] = AtlasId(0,0,3,0,2,0);
		atlas[MCube::MCubeType::CUBE_TERRE] = AtlasId(2,0,2,0,2,0);
		atlas[MCube::MCubeType::CUBE_PIERRE] = AtlasId(1,0,1,0,1,0);
		atlas[MCube::MCubeType::CUBE_EAU] = AtlasId(13,12, 13, 12, 13, 12);
		atlas[MCube::MCubeType::CUBE_SABLE_01] = AtlasId(2,1, 2, 1, 2, 1);
		atlas[MCube::MCubeType::CUBE_SABLE_02] = AtlasId(0,11, 0, 12, 0, 13);
	}
};