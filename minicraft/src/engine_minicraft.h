#ifndef __YOCTO__ENGINE_TEST__
#define __YOCTO__ENGINE_TEST__

#include "engine/engine.h"
#include "avatar.h"
#include "world.h"
#include <sysinfoapi.h>
#include "Inputs.h"
#include "world.h"


class MEngineMinicraft : public YEngine {

private:
	inline static MEngineMinicraft * instanceMinicraft;
public :
	//Gestion singleton
	static MEngineMinicraft * getInstance()
	{
		if (instanceMinicraft == NULL)
			instanceMinicraft = new MEngineMinicraft();
		if (Instance == NULL)
			Instance = instanceMinicraft;
		return instanceMinicraft;
	}

	//Mesh 
	YVbo * VboCube;
	MWorld * World;


	//Shader
	AtlasMap * atlas;
	int SunShader;
	int CubeShader;
	int CubeDebugShader;
	int WorldShader;

	// Click State
	bool rightMouseClick = false;
	bool wheelMouseClick = false;

	// Input State 
	Inputs inputs = Inputs();
	// Camera 
	float cameraSpeed = 1;


	/*HANDLERS GENERAUX*/
	void loadShaders() {
		//Chargement du shader (dans loadShaders() pour etre lié à F5)
		SunShader = Renderer->createProgram("shaders/sun");
		CubeShader = Renderer->createProgram("shaders/cube");
		CubeDebugShader = Renderer->createProgram("shaders/cube_debug");
		WorldShader = Renderer->createProgram("shaders/world");
	}

	void init()		
	{
		atlas = AtlasMap::GetInstance();
		YLog::log(YLog::ENGINE_INFO,"Minicraft Started : initialisation");

		Renderer->setBackgroundColor(YColor(0.0f,0.0f,0.0f,1.0f));
		Renderer->Camera->setPosition(YVec3f(60, 60, 60));
		Renderer->Camera->setLookAt(YVec3f(140,140,80));
		
		// Load Mesh
		VboCube = createGPUCube();
		loadShaders();

		//Pour créer le monde
		World = new MWorld();
		
		World->init_world(0);
		World->initCam(Renderer->Camera);

	}
	

	void update(float elapsed)
	{
		YVec3f moveVector = YVec3f(0, 0, 0);
		if (inputs.Z.press) // Z
		{
			moveVector.X += 1;
			//World->add_world_to_vbo();
		}
		if (inputs.S.press) // S
		{
			moveVector.X += -1;
		}
		if (inputs.Q.press) // Q
		{
			moveVector.Y += 1;
		}
		if (inputs.D.press) // D
		{
			moveVector.Y += -1;
		}

		if (moveVector.getSize() > 0) Renderer->Camera->relativeMove(moveVector.normalize() * elapsed * cameraSpeed * ((inputs.Shift.press) ? 50 : 10));
		World->updateCam();

		World->LoadVBO();
	}

	void renderObjects() 
	{
		float rotateTime = 0;
		SYSTEMTIME s;
		GetLocalTime(&s);
		
		rotateTime = s.wHour + s.wMinute/60;

		float calculTime = 0;
		if (rotateTime > 6 && rotateTime <= 19) calculTime = (rotateTime - 6) / 26;
		else if (rotateTime > 19) calculTime = (rotateTime - 19)/22 + 0.5f;
		else if (rotateTime <= 6) calculTime = (rotateTime + 5)/22 + 0.5f;

		YColor black = YColor(0, 0, 0, 1);
		YColor orange = YColor(1, 0.5f, 0, 1);
		YColor cyan = YColor(0, 1, 1, 1);

		Renderer->setBackgroundColor((calculTime <= 0.5f) ? cyan.interpolate(orange, std::abs(calculTime - 0.25f) / 0.25f) : black.interpolate(orange, std::abs(calculTime - 0.75f) / 0.25f));

		glUseProgram(0);
		//Rendu des axes
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3d(1, 0, 0);
		glVertex3d(0, 0, 0);
		glVertex3d(10000,0,0);
		glColor3d(0, 1, 0);
		glVertex3d(0, 0, 0);
		glVertex3d(0,10000,0);
		glColor3d(0, 0, 1);
		glVertex3d(0, 0, 0);
		glVertex3d(0,0, 10000);
		glEnd();

		glPushMatrix();
		glTranslatef(-10.5f, std::cos(calculTime *2*3.1415f)*10, std::sin(calculTime *2*3.1415f)*10);

		
		// SUN
		glUseProgram(SunShader); //Demande au GPU de charger ces shaders

		YColor red = YColor(1, 0, 0, 1);
		YColor white = YColor(1, 1, 0.5f, 1);
		YColor sunColor = (calculTime <=0.5f)?white.interpolate(red, std::abs(calculTime - 0.25f) / 0.25f):red;

		GLuint var = glGetUniformLocation(SunShader, "sun_color");
		glUniform3f(var, sunColor.R, sunColor.V, sunColor.B);

		Renderer->updateMatricesFromOgl(); //Calcule toute les matrices à partir des deux matrices OGL
		Renderer->sendMatricesToShader(SunShader); //Envoie les matrices au shader
		VboCube->render(); //Demande le rendu du VBO
		glPopMatrix();

		glPushMatrix();
		//World->render_world_basic(CubeShader,VboCube);
		World->render_world_vbo(WorldShader, atlas->terrain, false, false);
		glPopMatrix();

		
	}

	

	void resize(int width, int height) {
	
	}

	
	void keyPressed(int key, bool special, bool down, int p1, int p2) 
	{	
		inputs.keyPressed((((special) ? -1 : 1) * (key + ((inputs.Shift.press && !special) ? 32 : 0))), down, p1, p2);
	}

	void mouseWheel(int wheel, int dir, int x, int y, bool inUi)
	{
		Renderer->Camera->FovY += (wheel == 3) ? -1 : 1;
	}
	
	void mouseClick(int button, int state, int x, int y, bool inUi)
	{
		if (button == 1) {
			wheelMouseClick = state == 0;
		}
		else if (button == 2) {
			rightMouseClick = state == 0;
		}
		if(state == 0) CenterPointer();

	}
	void CenterPointer() {
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) >> 1, glutGet(GLUT_WINDOW_HEIGHT) >> 1);
	}

	void mouseMove(int x, int y, bool pressed, bool inUi)
	{
		x = x - (glutGet(GLUT_WINDOW_WIDTH) >> 1);
		y = y - (glutGet(GLUT_WINDOW_HEIGHT) >> 1);
		if (rightMouseClick) {
			if (inputs.Ctrl.press) {
				Renderer->Camera->rotateAround(x / 500.0f);
				Renderer->Camera->rotateUpAround(y / 500.0F);
			}
			else{
				Renderer->Camera->rotate(x/ 500.0f);
				Renderer->Camera->rotateUp(y / 500.0F);
			}
			CenterPointer();
		}
		else if (wheelMouseClick) {
			if (inputs.Ctrl.press) {

				Renderer->Camera->moveWorld(YVec3f( y / 500.0F, x / 500.0F, 0));
			}
			else {

				Renderer->Camera->relativeMove(YVec3f( y / 500.0F, x / 500.0F, 0));
			}
			CenterPointer();
		}
	}
	

	YVbo * createGPUCube() {
		// Creation du VBO
		YVbo * VboCube = new YVbo(3, 36, YVbo::PACK_BY_ELEMENT_TYPE);

		// Définition du contenu VBO
		VboCube->setElementDescription(0, YVbo::Element(3)); //Sommet
		VboCube->setElementDescription(1, YVbo::Element(3)); //Normale
		VboCube->setElementDescription(2, YVbo::Element(2)); //UV

		VboCube->createVboCpu();

		// On ajoute les sommets
		int iVertice = 0;

		// Surface 1
		VboCube->setElementValue(0, iVertice, 0, 0, 0);
		VboCube->setElementValue(1, iVertice, 0, -1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 0, 0);
		VboCube->setElementValue(1, iVertice, 0, -1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 0, 1);
		VboCube->setElementValue(1, iVertice, 0, -1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		// Surface 1
		VboCube->setElementValue(0, iVertice, 1, 0, 0);
		VboCube->setElementValue(1, iVertice, 0, -1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 0, 1);
		VboCube->setElementValue(1, iVertice, 0, -1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 0, 1);
		VboCube->setElementValue(1, iVertice, 0, -1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;

		// Surface 2
		VboCube->setElementValue(0, iVertice, 0, 0, 0);
		VboCube->setElementValue(1, iVertice, -1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 0, 1);
		VboCube->setElementValue(1, iVertice, -1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 1, 0);
		VboCube->setElementValue(1, iVertice, -1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		// Surface 2
		VboCube->setElementValue(0, iVertice, 0, 1, 0);
		VboCube->setElementValue(1, iVertice, -1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 0, 1);
		VboCube->setElementValue(1, iVertice, -1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 1, 1);
		VboCube->setElementValue(1, iVertice, -1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;


		// Surface 3
		VboCube->setElementValue(0, iVertice, 0, 0, 0);
		VboCube->setElementValue(1, iVertice, 0, 0, -1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 1, 0);
		VboCube->setElementValue(1, iVertice, 0, 0, -1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 0, 0);
		VboCube->setElementValue(1, iVertice, 0, 0, -1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		// Surface 3
		VboCube->setElementValue(0, iVertice, 1, 0, 0);
		VboCube->setElementValue(1, iVertice, 0, 0, -1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 1, 0);
		VboCube->setElementValue(1, iVertice, 0, 0, -1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 1, 0);
		VboCube->setElementValue(1, iVertice, 0, 0, -1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;


		// Surface 1
		VboCube->setElementValue(0, iVertice, 1, 1, 0);
		VboCube->setElementValue(1, iVertice, 0, 1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 1, 0);
		VboCube->setElementValue(1, iVertice, 0, 1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 1, 1);
		VboCube->setElementValue(1, iVertice, 0, 1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		// Surface 1
		VboCube->setElementValue(0, iVertice, 0, 1, 1);
		VboCube->setElementValue(1, iVertice, 0, 1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 1, 1);
		VboCube->setElementValue(1, iVertice, 0, 1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 1, 0);
		VboCube->setElementValue(1, iVertice, 0, 1, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;


		// Surface 2
		VboCube->setElementValue(0, iVertice, 1, 0, 0);
		VboCube->setElementValue(1, iVertice, 1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 1, 0);
		VboCube->setElementValue(1, iVertice, 1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 0, 1);
		VboCube->setElementValue(1, iVertice, 1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		// Surface 2
		VboCube->setElementValue(0, iVertice, 1, 1, 0);
		VboCube->setElementValue(1, iVertice, 1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 1, 1);
		VboCube->setElementValue(1, iVertice, 1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 0, 1);
		VboCube->setElementValue(1, iVertice, 1, 0, 0);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;


		// Surface 3
		VboCube->setElementValue(0, iVertice, 0, 0, 1);
		VboCube->setElementValue(1, iVertice, 0, 0, 1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 0, 1);
		VboCube->setElementValue(1, iVertice, 0, 0, 1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 1, 1);
		VboCube->setElementValue(1, iVertice, 0, 0, 1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		// Surface 3
		VboCube->setElementValue(0, iVertice, 1, 0, 1);
		VboCube->setElementValue(1, iVertice, 0, 0, 1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 1, 1, 1);
		VboCube->setElementValue(1, iVertice, 0, 0, 1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;
		VboCube->setElementValue(0, iVertice, 0, 1, 1);
		VboCube->setElementValue(1, iVertice, 0, 0, 1);
		VboCube->setElementValue(2, iVertice, 0, 0);
		++iVertice;

		// On envoie le contenu au GPU
		VboCube->createVboGpu();
		// On clean le contenu du CPU
		VboCube->deleteVboCpu();


		return VboCube;

	}
};


#endif