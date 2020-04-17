#ifndef __YOCTO__ENGINE_TEST__
#define __YOCTO__ENGINE_TEST__

#include "engine/engine.h"
#include "engine/noise/perlin.h"

class YEngineTest : public YEngine {
	YVbo * Vbo;
	YVbo * VboRand;
	YVbo * VboPlane;
	GLuint ShaderTest1 = 0;
	GLuint PostProcessProg = 0;
	YPerlin * Perlin;
	YFbo * Fbo;
public :		
	//Gestion singleton
	static YEngine * getInstance()
	{
		if (Instance == NULL)
			Instance = new YEngineTest();
		return Instance;
	}

	void init() 
	{
		YLog::log(YLog::ENGINE_INFO, "EngineTest Started");

		//On place la camera
		Renderer->Camera->setPosition(YVec3f(-5,-5,5));
		Renderer->Camera->setLookAt(YVec3f(5,5,0)); 

		//On cree le FBO pour le post process
		Fbo = new YFbo(1);
		Fbo->init(Renderer->ScreenWidth, Renderer->ScreenHeight);

		//On crée les materials
		ShaderTest1 = Renderer->createProgram("shaders/test1");
		PostProcessProg = Renderer->createProgram("shaders/postprocess");
		
		Vbo = new YVbo(3,6, YVbo::PACK_BY_VERTICE); //Deux Triangles, avec sommet, couleur et normale
		Vbo->setElementDescription(0, YVbo::Element(3)); //Sommet
		Vbo->setElementDescription(1, YVbo::Element(4)); //Couleur
		Vbo->setElementDescription(2, YVbo::Element(3)); //Normale
		Vbo->createVboCpu();

		//3 sommets
		Vbo->setElementValue(0, 0, 0, 0, 0);
		Vbo->setElementValue(0, 1, 1, 0, 0);
		Vbo->setElementValue(0, 2, 0, 1, 0);

		//Couleurs
		Vbo->setElementValue(1, 0, 1, 0, 0, 1);
		Vbo->setElementValue(1, 1, 0, 1, 0, 1);
		Vbo->setElementValue(1, 2, 0, 0, 1, 1);

		//3 normales
		Vbo->setElementValue(2, 0, 0, 0, 1);
		Vbo->setElementValue(2, 1, 0, 0, 1);
		Vbo->setElementValue(2, 2, 0, 0, 1);

		Vbo->createVboGpu();
		Vbo->deleteVboCpu();

		//On cree le VBO random
		VboRand = new YVbo(3, 1000000, YVbo::PACK_BY_ELEMENT_TYPE); //Deux Triangles, avec sommet, couleur et normale
		VboRand->setElementDescription(0, YVbo::Element(3)); //Sommet
		VboRand->setElementDescription(1, YVbo::Element(4)); //Couleur
		VboRand->setElementDescription(2, YVbo::Element(3)); //Normales
		VboRand->createVboCpu();

		for (int i = 0; i < VboRand->getNbVertices(); i++) {
			VboRand->setElementValue(0, i, (float)randf(), (float)randf(), (float)randf());
			VboRand->setElementValue(1, i, (float)randf(), (float)randf(), (float)randf(), 1);
			VboRand->setElementValue(2, i, (float)randf(), (float)randf(), (float)randf());
		}

		VboRand->createVboGpu();
		VboRand->deleteVboCpu();

		//On cree le VBO plane 
		int planeSize = 100;
		VboPlane = new YVbo(3, planeSize*planeSize*6, YVbo::PACK_BY_VERTICE); 
		VboPlane->setElementDescription(0, YVbo::Element(3)); //Sommet
		VboPlane->setElementDescription(1, YVbo::Element(4)); //Couleur
		VboPlane->setElementDescription(2, YVbo::Element(3)); //Normale
		VboPlane->createVboCpu();

		//On cree notre perlin
		Perlin = new YPerlin();
		Perlin->setFreq(0.08f);

		//Pour chaque point, on output 2 triangles qui font le quad
		for (int x = 0; x < planeSize; x++) {
			for (int y = 0; y < planeSize; y++) {

				//Premier triangle 
				VboPlane->setElementValue(0, (y*planeSize + x) * 6 + 0, x, y, 10 * Perlin->sample(x,y,1)-10);
				VboPlane->setElementValue(1, (y*planeSize + x) * 6 + 0, 0.5f, 0.5f + randf()*0.3f, 0.5f, 1);
				VboPlane->setElementValue(2, (y*planeSize + x) * 6 + 0, 0, 0, 1);
				VboPlane->setElementValue(0, (y*planeSize + x) * 6 + 1, x + 1, y, 10 * Perlin->sample(x+1, y, 1) - 10);
				VboPlane->setElementValue(1, (y*planeSize + x) * 6 + 1, 0.5f, 0.5f + randf()*0.3f, 0.5f, 1);
				VboPlane->setElementValue(2, (y*planeSize + x) * 6 + 1, 0, 0, 1);
				VboPlane->setElementValue(0, (y*planeSize + x) * 6 + 2, x, y + 1, 10 * Perlin->sample(x, y + 1, 1) - 10);
				VboPlane->setElementValue(1, (y*planeSize + x) * 6 + 2, 0.5f, 0.5f + randf()*0.3f, 0.5f, 1);
				VboPlane->setElementValue(2, (y*planeSize + x) * 6 + 2, 0, 0, 1);

				//Second triangle 
				VboPlane->setElementValue(0, (y*planeSize + x) * 6 + 3, x, y + 1, 10 * Perlin->sample(x, y + 1, 1) - 10);
				VboPlane->setElementValue(1, (y*planeSize + x) * 6 + 3, 0.5f, 0.5f + randf()*0.3f, 0.5f, 1);
				VboPlane->setElementValue(2, (y*planeSize + x) * 6 + 3, 0, 0, 1);
				VboPlane->setElementValue(0, (y*planeSize + x) * 6 + 4, x + 1, y, 10 * Perlin->sample(x + 1, y, 1) - 10);
				VboPlane->setElementValue(1, (y*planeSize + x) * 6 + 4, 0.5f, 0.5f + randf()*0.3f, 0.5f, 1);
				VboPlane->setElementValue(2, (y*planeSize + x) * 6 + 4, 0, 0, 1);
				VboPlane->setElementValue(0, (y*planeSize + x) * 6 + 5, x + 1, y + 1, 10*Perlin->sample(x + 1, y + 1, 1) - 10);
				VboPlane->setElementValue(1, (y*planeSize + x) * 6 + 5, 0.5f, 0.5f + randf()*0.3f, 0.5f, 1);
				VboPlane->setElementValue(2, (y*planeSize + x) * 6 + 5, 0, 0, 1);
			}
		}

		VboPlane->createVboGpu();
		VboPlane->deleteVboCpu();
	}

	void update(float elapsed) 
	{
		
	}

	void renderObjects() 
	{
		Fbo->setAsOutFBO(true);

		//Rendu des axes
		glUseProgram(0);
		//YEngine<void>::TimerGPURender.startAccumPeriod();
		glBegin(GL_LINES);
		glColor3d(1, 0, 0);
		glVertex3d(0, 0, 0);
		glVertex3d(10000, 0, 0);
		glColor3d(0, 1, 0);
		glVertex3d(0, 0, 0);
		glVertex3d(0, 10000, 0);
		glColor3d(0, 0, 1);
		glVertex3d(0, 0, 0);
		glVertex3d(0, 0, 10000);
		glEnd();
		//YEngine<void>::TimerGPURender.endAccumPeriod();

		glUseProgram(ShaderTest1);
		
		Renderer->updateMatricesFromOgl();
		Renderer->sendMatricesToShader(ShaderTest1);

		Vbo->render();
		VboPlane->render();

		Fbo->setAsOutFBO(false);

		glUseProgram(PostProcessProg);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		Fbo->setColorAsShaderInput(0, GL_TEXTURE0,"TexColor");
		Fbo->setDepthAsShaderInput(GL_TEXTURE1, "TexDepth");

		Renderer->sendNearFarToShader(PostProcessProg);
		Renderer->drawFullScreenQuad();


	}

	void resize(int width, int height) 
	{
		Fbo->resize(width, height);
	}

	void keyPressed(int key, bool special, bool down, int p1, int p2) {

	}

	void mouseWheel(int wheel, int dir, int x, int y, bool inUi)
	{
		Renderer->Camera->move(Renderer->Camera->Direction * 10.0f * (float)dir);
	}

	void mouseClick(int button, int state, int x, int y, bool inUi)
	{
		
	}

	void mouseMove(int x, int y, bool pressed, bool inUi)
	{
		
	}
	
};


#endif