#ifndef __YOCTO__ENGINE__
#define __YOCTO__ENGINE__

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"

#include "engine/utils/types_3d.h"
#include "engine/utils/timer.h"
#include "engine/log/log_console.h"
#include "engine/render/Renderer.h"
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"
#include "engine/render/tex_manager.h"

class YEngine {

public:
	inline static YEngine * Instance; //Singleton
	YRenderer * Renderer = NULL;
		
	//Temps
	YTimer * Timer = NULL;
	YTimer TimerGPURender;
	inline static float GPUPartTime = 0;
	float FpsElapsed = 0.0f;
	float FpsNbFrames = 0;
	inline static float DeltaTime = 0.0f; ///< Temps écoulé depuis la dernière frame (passe a la fonction render)
	inline static float DeltaTimeCumul = 0.0f; ///< Temps écoulé depuis le lancement de l'appli

	//Rendu
	float NearPlane = 0.1f;
	float FarPlane = 800.0f;
	float Fov = 45.0f;
		
	//Window
	int MainWindowId;
	bool MouseVisible = true;
	bool FullScreen = false;
	const int BaseWidth = 800;
	const int BaseHeight = 600;

	//GUI 
	int MouseBtnState = 0;
	GUIScreenManager * ScreenManager = NULL;
	GUIScreen * ScreenParams = NULL;
	GUIScreen * ScreenStats = NULL;
	GUIScreen * ScreenJeu = NULL;
	GUILabel * LblFps = NULL;
		
protected :
	YEngine() {
			
	}

public :

	//Gestion singleton
	static YEngine * getInstance()
	{
		if (Instance == NULL)
			Instance = new YEngine();
		return Instance;
	}

	void initBase(int argc, char* argv[]) {

		YLogConsole::createInstance();
			
		glutInit(&argc, argv);
		glutSetOption(
			GLUT_ACTION_ON_WINDOW_CLOSE,
			GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);
		
		glutInitWindowSize(BaseWidth, BaseHeight);
		glutInitWindowPosition(0, 0);
		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

		YLog::log(YLog::ENGINE_INFO, (toString(argc) + " arguments en ligne de commande.").c_str());
		FullScreen = false;
		for (int i = 0; i<argc; i++)
		{
			if (argv[i][0] == 'f')
			{
				YLog::log(YLog::ENGINE_INFO, "Arg f mode fullscreen.\n");
				FullScreen = true;
			}
		}

		MainWindowId = glutCreateWindow("Yocto");
		glutReshapeWindow(BaseWidth, BaseHeight);
		setFullScreen(FullScreen);

		if (MainWindowId < 1)
		{
			YLog::log(YLog::ENGINE_ERROR, "Erreur creation de la fenetre.");
			exit(EXIT_FAILURE);
		}

		GLenum glewInitResult = glewInit();
		if (glewInitResult != GLEW_OK)
		{
			YLog::log(YLog::ENGINE_ERROR, ("Erreur init glew " + std::string((char*)glewGetErrorString(glewInitResult))).c_str());
			exit(EXIT_FAILURE);
		}

		//Affichage des capacités du système
		YLog::log(YLog::ENGINE_INFO, ("OpenGL Version : " + std::string((char*)glGetString(GL_VERSION))).c_str());

		glutDisplayFunc(updateBase);
		glutReshapeFunc(resizeBase);
		glutKeyboardFunc(keyboardDown);
		glutKeyboardUpFunc(keyboardUp);
		glutSpecialFunc(specialDown);
		glutSpecialUpFunc(specialUp);
		glutMouseFunc(mouseClick);
		glutMotionFunc(mouseMoveActive);
		glutPassiveMotionFunc(mouseMovePassive);
		glutIgnoreKeyRepeat(1);

		//Initialisation du YRenderer
		Renderer = YRenderer::getInstance();
		Renderer->setRenderObjectFun(renderObjectsBase);
		Renderer->setRender2DFun(render2dBase);
		Renderer->setBackgroundColor(YColor());
		Renderer->initialise(&TimerGPURender);

		//On applique la config du YRenderer
		glViewport(0, 0, Renderer->ScreenWidth, Renderer->ScreenHeight);
		Renderer->resize(Renderer->ScreenWidth, Renderer->ScreenHeight);
			
		//Ecrans de jeu
		ScreenManager = new GUIScreenManager();
		uint16 x = 10;
		uint16 y = 10;
		ScreenJeu = new GUIScreen();
		ScreenStats = new GUIScreen();

		//Bouton pour afficher les params
		GUIBouton * btn = new GUIBouton();
		btn->Titre = std::string("Params");
		btn->X = x;
		btn->Y = y;
		btn->setOnClick(clickBtnParams);
		ScreenJeu->addElement(btn);

		y += btn->Height + 5;

		btn = new GUIBouton();
		btn->Titre = std::string("Stats");
		btn->X = x;
		btn->Y = y;
		btn->setOnClick(clickBtnStats);
		ScreenJeu->addElement(btn);

		y += btn->Height + 1;

		//Ecran de stats
		y = btn->Height + 15;

		LblFps = new GUILabel();
		LblFps->Text = "FPS";
		LblFps->X = x;
		LblFps->Y = y;
		LblFps->Visible = true;
		ScreenStats->addElement(LblFps);

		//Ecran de parametrage
		x = 10;
		y = 10;
		ScreenParams = new GUIScreen();

		GUIBouton * btnClose = new GUIBouton();
		btnClose->Titre = std::string("Close");
		btnClose->X = x;
		btnClose->Y = y;
		btnClose->setOnClick(clickBtnClose);
		ScreenParams->addElement(btnClose);
		ScreenStats->addElement(btnClose);

		//Ecran a rendre
		ScreenManager->setActiveScreen(ScreenJeu);

		//Init YCamera
		Renderer->Camera->setPosition(YVec3f(320, 320, 320));
		Renderer->Camera->setLookAt(YVec3f(0, 0, 0));
		Renderer->Camera->setProjectionPerspective(Instance->Fov,
			(float)Instance->Renderer->ScreenWidth / (float)Instance->Renderer->ScreenHeight,
			Instance->NearPlane, Instance->FarPlane);
			
		//Init YTimer
		Timer = new YTimer();

		//Chargement des shaders
		Instance->loadShaders();

		//Init pour classe fille
		init();

		//On start le temps
		Timer->start();
		
		YLog::log(YLog::ENGINE_INFO, "[   Yocto initialized   ]\nPress : \n - f to toggle fullscreen\n - F1 for png screen shot\n - F5 to hot-reload shaders");

	}
		
	static void updateBase(void) {
		float elapsed = Instance->Timer->getElapsedSeconds(true);

		DeltaTime = elapsed;
		DeltaTimeCumul += elapsed;

		//Calcul des fps
		Instance->FpsElapsed += elapsed;
		Instance->FpsNbFrames++;
		if (Instance->FpsElapsed > 1.0)
		{
			Instance->GPUPartTime = Instance->TimerGPURender.getAccumTimeSec() / Instance->FpsElapsed;
			float GPUPcent = floor(Instance->GPUPartTime * 100);
			Instance->LblFps->Text = std::string("FPS : ") +
				toString(Instance->FpsNbFrames) +
				string(" ") + toString(GPUPcent) +
				string("/100 in GPU, ") +
				toString(YRenderer::NbVBOFacesRendered/1000) + string(" Kfaces");
			Instance->FpsElapsed -= 1.0f;
			Instance->FpsNbFrames = 0;
			Instance->TimerGPURender.resetAccumPeriod();
		}

		//On update tout
		Instance->update(elapsed);

		//Et on dessine
		Instance->Renderer->render(elapsed);		
	}

	static void render2dBase(void)
	{
		Instance->ScreenManager->render();
	}

	static void renderObjectsBase(void)
	{
		Instance->renderObjects();
	}

	static void resizeBase(int width, int height)
	{
		glViewport(0,0, width, height);
		Instance->Renderer->resize(width, height);
		Instance->Renderer->Camera->setProjectionPerspective(Instance->Fov, 
			(float)Instance->Renderer->ScreenWidth / (float)Instance->Renderer->ScreenHeight, 
			Instance->NearPlane, Instance->FarPlane);
		Instance->resize(width, height);
	}

	void setFullScreen(bool full)
	{
		FullScreen = full;
		if (FullScreen) {
			glutFullScreen();
		}
		else {
			glutLeaveFullScreen();
			glutReshapeWindow(BaseWidth, BaseHeight);
		}
	}

	void showMouse(bool show)
	{
		Instance->MouseVisible = show;
		if (Instance->MouseVisible)
			glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
		else
			glutSetCursor(GLUT_CURSOR_NONE);
	}

	///////////////////////////////////////////////////////////////////////////////
	//  GESTION DES E/S
	///////////////////////////////////////////////////////////////////////////////
	static void specialDown(int key, int p1, int p2)
	{
		if (key == GLUT_KEY_F5) {
			Instance->loadShaders();
		}

		if (key == GLUT_KEY_F1) {
			SYSTEMTIME t;
			GetLocalTime(&t);
			Instance->Renderer->screenShot((
				toString(t.wYear) + "_" +
				toString(t.wMonth) + "_" +
				toString(t.wDay) + "_" +
				toString(t.wHour) + "_" +
				toString(t.wMinute) + "_" +
				toString(t.wSecond) + "_" +
				toString(t.wMilliseconds) + ".png").c_str());
		}

		Instance->keyPressed(key, true, true, p1, p2);
	}

	static void specialUp(int key, int p1, int p2)
	{
		Instance->keyPressed(key, true, false, p1, p2);
	}

	static void keyboardDown(unsigned char key, int p1, int p2)
	{
		if (key == VK_ESCAPE)
		{
			glutDestroyWindow(Instance->MainWindowId);
			exit(0);
		}

		if (key == 'f')
		{
			Instance->setFullScreen(!Instance->FullScreen);
		}

		Instance->keyPressed(key, false, true, p1, p2);
	}

	static void keyboardUp(unsigned char key, int p1, int p2)
	{
		Instance->keyPressed(key, false, false, p1, p2);
	}

	static void mouseWheel(int wheel, int dir, int x, int y)
	{
		Instance->mouseWheel(wheel, dir, x, y, false);
	}
		
	static void mouseClick(int button, int state, int x, int y)
	{
		//Gestion de la roulette de la souris
		if ((button & 0x07) == 3 && state)
			mouseWheel(button, 1, x, y);
		if ((button & 0x07) == 4 && state)
			mouseWheel(button, -1, x, y);

		//Update current mouse state
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
			Instance->MouseBtnState |= GUI_MLBUTTON;
		if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
			Instance->MouseBtnState |= GUI_MRBUTTON;
		if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
			Instance->MouseBtnState |= GUI_MMBUTTON;
		if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
			Instance->MouseBtnState &= ~GUI_MRBUTTON;
		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
			Instance->MouseBtnState &= ~GUI_MLBUTTON;
		if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
			Instance->MouseBtnState &= ~GUI_MMBUTTON;

		bool mouseTraite = false;
		mouseTraite = Instance->ScreenManager->mouseCallback(x, y, Instance->MouseBtnState, 0, 0);
		Instance->mouseClick(button,state,x,y, mouseTraite);
	}

	static void mouseMove(int x, int y, bool pressed)
	{
		bool mouseTraite = false;
		mouseTraite = Instance->ScreenManager->mouseCallback(x, y, Instance->MouseBtnState, 0, 0);
		Instance->mouseMove(x, y, pressed, mouseTraite);
	}

	static void mouseMoveActive(int x, int y) {	mouseMove(x, y, true); }
	static void mouseMovePassive(int x, int y) { mouseMove(x, y, false); }

	static void clickBtnParams(GUIBouton * bouton) 
	{ 
		Instance->ScreenManager->setActiveScreen(Instance->ScreenParams);
	}

	static void clickBtnStats(GUIBouton * bouton)
	{
		Instance->ScreenManager->setActiveScreen(Instance->ScreenStats);
	}

	static void clickBtnClose(GUIBouton * bouton)
	{
		Instance->ScreenManager->setActiveScreen(Instance->ScreenJeu);
	}

public:
	///////////////////////////////////////////////////////////////////////////////
	//  CALLBACKS DERIVABLES
	///////////////////////////////////////////////////////////////////////////////
	virtual void init() {}
	virtual void update(float elapsed) {}
	virtual void keyPressed(int key, bool special, bool down, int p1, int p2) {}
	virtual void mouseWheel(int wheel, int dir, int x, int y, bool inUi) {}
	virtual void mouseClick(int button, int state, int x, int y, bool inUi)	{}
	virtual void mouseMove(int x, int y, bool pressed, bool inUi){}
	virtual void renderObjects(){}
	virtual void resize(int width, int height) {}
	virtual void loadShaders() {}
};

#endif