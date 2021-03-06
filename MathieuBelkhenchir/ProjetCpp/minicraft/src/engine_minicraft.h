#ifndef __YOCTO__ENGINE_TEST__
#define __YOCTO__ENGINE_TEST__

#include "engine/engine.h"
#include "avatar.h"
#include "world.h"
#include <sysinfoapi.h>
#include "Inputs.h"
#include "world.h"
#include "avatar.h"
#include "m_physics.h"

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
	YFbo * Fbo;
	MWorld * World;
	YCamera * main;
	YCamera * light;

	//Shader
	AtlasMap * atlas;
	MAvatar * avatar;
	int SunShader;
	int CubeShader;
	int CubeDebugShader;
	int WorldShader;
	int PostProcessProg;


	// Click State
	bool rightMouseClick = false;
	bool wheelMouseClick = false;

	// Input State 
	Inputs * inputs = new Inputs();
	// Camera 
	float cameraSpeed = 1;


	/*HANDLERS GENERAUX*/
	void loadShaders() {
		//Chargement du shader (dans loadShaders() pour etre li� � F5)
		SunShader = Renderer->createProgram("shaders/sun");
		CubeShader = Renderer->createProgram("shaders/cube");
		CubeDebugShader = Renderer->createProgram("shaders/cube_debug");
		WorldShader = Renderer->createProgram("shaders/world");
		PostProcessProg = Renderer->createProgram("shaders/postprocess");
	}

	void init()		
	{
		Fbo = new YFbo(1);
		Fbo->init(Renderer->ScreenWidth, Renderer->ScreenHeight);


		atlas = AtlasMap::GetInstance();
		YLog::log(YLog::ENGINE_INFO,"Minicraft Started : initialisation");

		Renderer->setBackgroundColor(YColor(0.0f,0.0f,0.0f,1.0f));
		Renderer->Camera->setPosition(YVec3f(60, 60, 80));
		Renderer->Camera->setLookAt(YVec3f(140,140,80));
		main = Renderer->Camera;
		light = new YCamera();
		// Load Mesh
		VboCube = createGPUCube();
		loadShaders();

		//Pour cr�er le monde
		World = new MWorld();
		avatar = new MAvatar(Renderer->Camera,World,inputs);
		World->init_world(0);
		World->initCam(Renderer->Camera);


	}
	/*
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
	*/
	float totalTime = 0;
	void update(float elapsed)
	{
		totalTime += elapsed;
		/*
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
		*/
		avatar->update(elapsed);
		World->updateCam();
		World->LoadVBO();
	}

	float incrHour = 0;
	void renderObjects() 
	{
		
		float rotateTime = 0;
		SYSTEMTIME s;
		GetLocalTime(&s);
		
		rotateTime = s.wHour + s.wMinute/60 + incrHour;
		if (rotateTime > 24) rotateTime -= 24;
		//rotateTime = ((s.wSecond+s.wMilliseconds/1000.0f) / 60.0f) * 24 + incrHour;
		float calculTime = 0;
		if (rotateTime > 6 && rotateTime <= 19) calculTime = (rotateTime - 6) / 26;
		else if (rotateTime > 19) calculTime = (rotateTime - 19)/22 + 0.5f;
		else if (rotateTime <= 6) calculTime = (rotateTime + 5)/22 + 0.5f;

		YColor black = YColor(0, 0, 0, 1);
		YColor orange = YColor(1, 0.5f, 0, 1);
		YColor cyan = YColor(0, 1, 1, 1);
		YColor skyColor = (calculTime <= 0.5f) ? cyan.interpolate(orange, std::abs(calculTime - 0.25f) / 0.25f) : black.interpolate(orange, std::abs(calculTime - 0.75f) / 0.25f);
		Renderer->setBackgroundColor(skyColor);

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
		glColor3d(1, 0, 0);
		glEnd();
		
		Fbo->setAsOutFBO(true);
		glPushMatrix();
		YVec3f sunpos = YVec3f(-100, std::cos(calculTime * 2 * 3.1415f) * 500, std::sin(calculTime * 2 * 3.1415f) * 500).normalize();
		glUseProgram(SunShader); //Demande au GPU de charger ces shaders

		glTranslatef(sunpos.X * 200 + avatar->Position.X, sunpos.Y * 200 + avatar->Position.Y, sunpos.Z * 200 + avatar->Position.Z);
		glTranslatef(10, 10, 10);
		glScalef(20, 20, 20);
		
		// SUN
		YColor red = YColor(1, 0, 0, 1);
		YColor white = YColor(1, 1, 0.5f, 1);
		YColor sunColor = (calculTime <=0.5f)?white.interpolate(red, std::abs(calculTime - 0.25f) / 0.25f):red;

		GLuint var = glGetUniformLocation(SunShader, "sun_color");
		glUniform3f(var, sunColor.R, sunColor.V, sunColor.B);

		Renderer->updateMatricesFromOgl(); //Calcule toute les matrices � partir des deux matrices OGL
		Renderer->sendMatricesToShader(SunShader); //Envoie les matrices au shader
		VboCube->render(); //Demande le rendu du VBO
		glPopMatrix();

		glPushMatrix();

		// AVATAR

		glPushMatrix();
		glTranslatef(avatar->Position.X - avatar->Width / 2, avatar->Position.Y - avatar->Width / 2, avatar->Position.Z - avatar->CurrentHeight / 2);
		glScalef(avatar->Width, avatar->Width, avatar->CurrentHeight);
		glUseProgram(CubeShader);
		Renderer->updateMatricesFromOgl();
		Renderer->sendMatricesToShader(CubeShader);
		var = glGetUniformLocation(CubeShader, "cube_color");
		glUniform4f(var, 1, 1, 1, 1.0f);
		VboCube->render();
		glPopMatrix();
		
		// WORLD
		glUseProgram(WorldShader);
		var = glGetUniformLocation(WorldShader, "cam_pos");
		glUniform3f(var, Renderer->Camera->Position.X, Renderer->Camera->Position.Y, Renderer->Camera->Position.Z);
		var = glGetUniformLocation(WorldShader, "sun_color");
		glUniform4f(var, sunColor.R, sunColor.V, sunColor.B, 1);
		World->render_world_vbo(WorldShader, atlas->terrain, false, false,sunpos,avatar->Position, totalTime);
		glPopMatrix();


		
		// PICK
		if (avatar->find) {
			glPushMatrix();

			glTranslatef(avatar->pickPos.X - 0.01f, avatar->pickPos.Y - 0.01f, avatar->pickPos.Z - 0.01f);
			glScalef(1.02f, 1.02f, 1.02f);
			glUseProgram(CubeShader);
			Renderer->updateMatricesFromOgl();
			Renderer->sendMatricesToShader(CubeShader);
			var = glGetUniformLocation(CubeShader, "cube_color");
			glUniform4f(var, 1, 0, 0, 1.0f);
			VboCube->render();
			glPopMatrix();
		}

		// POSTPROCESS
		Fbo->setAsOutFBO(false);

		
		
		glUseProgram(PostProcessProg);
		Renderer->sendScreenSizeToShader(PostProcessProg);
		var = glGetUniformLocation(PostProcessProg, "sky_color");
		glUniform4f(var, skyColor.R, skyColor.V, skyColor.B, 1);
		//glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		
		Fbo->setColorAsShaderInput(0, GL_TEXTURE0, "TexColor");
		Fbo->setDepthAsShaderInput(GL_TEXTURE1, "TexDepth");
		
		
		Renderer->sendNearFarToShader(PostProcessProg);
		Renderer->drawFullScreenQuad();
	}

	void resize(int width, int height) {
		Fbo->resize(width, height);
	}

	
	void keyPressed(int key, bool special, bool down, int p1, int p2) 
	{	
		inputs->keyPressed((((special) ? -1 : 1) * (key + ((inputs->Shift.press && key!=32 && !special) ? 32 : 0))), down, p1, p2);
		if (key == 3 && special && down)avatar->fps = !avatar->fps;
		if(key == 2 && down) ++incrHour;
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
		else {
			if (state == 0 && avatar->find) {
				World->deleteCube(avatar->pickPos.X, avatar->pickPos.Y, avatar->pickPos.Z);
			}
		}
		if (state == 0) {
			CenterPointer();
		}
	}
	void CenterPointer() {
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) >> 1, glutGet(GLUT_WINDOW_HEIGHT) >> 1);
	}

	void mouseMove(int x, int y, bool pressed, bool inUi)
	{
		x = x - (glutGet(GLUT_WINDOW_WIDTH) >> 1);
		y = y - (glutGet(GLUT_WINDOW_HEIGHT) >> 1);
		if (rightMouseClick) {
			if (inputs->Ctrl.press) {
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
			if (inputs->Ctrl.press) {

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

		// D�finition du contenu VBO
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