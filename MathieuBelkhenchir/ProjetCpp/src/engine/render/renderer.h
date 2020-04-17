#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "engine/render/camera.h" 
#include "engine/render/text_engine.h"
#include "engine/log/log.h"
#include "engine/render/fbo.h"
#include "engine/render/vbo.h"
#include "engine/utils/timer.h"

//BACKGROUND COLOR FOND
#define ROUGE_FOND 176.0f/255.0f
#define VERT_FOND 192.0f/255.0f
#define BLEU_FOND 198.0f/255.0f

#define DEFAULT_SCREEN_WIDTH 1200
#define DEFAULT_SCREEN_HEIGHT 800

class YRenderer
{
public:
	YCamera * Camera; ///< Gestion du point de vue
	int ScreenWidth; ///< Largeur ecran en pixels
	int ScreenHeight; ///< Hauteur ecran en pixels
	YColor BackGroundColor; ///< Couleur de fond. La modifier avec setBackgroundColor()
	YTextEngine * TextEngine; ///< Rendu de texte
	static const int CURRENT_SHADER = 0;
	inline static int NbVBOFacesRendered; ///< Nombre de faces rendues par les VBO, incrémentées par eux a chaque frames 

private:
	inline static YRenderer * _Me; ///< Singleton

	HWND _WHnd; ///< Handle de la fenetre principale
	YVbo * _VBOQuadFS; ///< le vbo pour dessiner un quad full screen

	YTimer * TimerGPU = NULL;

	void(*_RenderObjectsFun)(void); ///< Fonction de rendu des objets uniquement (parfois fait n fois dans certains rendus)
	void(*_Render2DFun)(void); ///< Rendu en 2d (en passe en mode camera ortho, etc...)

	//Matrices sauvées, a passer aux shaders, calculées depuis les matrices OGL
	YMat44 MatM;
	YMat44 MatV;
	YMat44 MatP;
	YMat44 MatNorm;
	YMat44 MatMV;
	YMat44 MatMVP;
	YMat44 MatIM;
	YMat44 MatIV;
	YMat44 MatIP;

	bool TakeScreenShot = false;
	string NameScreenShot;

	YRenderer()
	{
		Camera = new YCamera();
		Camera->setPosition(YVec3f(0, -190, 0));
		Camera->setLookAt(YVec3f(0, 0, 0));
		ScreenWidth = DEFAULT_SCREEN_WIDTH;
		ScreenHeight = DEFAULT_SCREEN_HEIGHT;
		_RenderObjectsFun = NULL;
		_Render2DFun = NULL;
		_WHnd = WindowFromDC(wglGetCurrentDC());
		TextEngine = new YTextEngine(wglGetCurrentDC());
		TextEngine->buildFont(0, 12);
		TextEngine->buildFont(1, 14);
		TextEngine->SelectFont(0);
		BackGroundColor.R = ROUGE_FOND;
		BackGroundColor.V = VERT_FOND;
		BackGroundColor.B = BLEU_FOND;
	}

public:

	static YRenderer * getInstance()
	{
		if (_Me == NULL)
			_Me = new YRenderer();
		return _Me;
	}

	/**
		* Changement de camera (pour une sous classe par exemple)
		*/
	void setCam(YCamera * cam)
	{
		Camera = cam;
	}

	/**
		* Initialisation du moteur
		*/
	void initialise(YTimer * timerGPU = NULL)
	{
		glClearColor(BackGroundColor.R, BackGroundColor.V, BackGroundColor.B, BackGroundColor.A);
		glDisable(GL_LIGHTING);

		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);

		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glEnable(GL_BLEND);

		glEnable(GL_CULL_FACE);

		_VBOQuadFS = new YVbo(1, 6, YVbo::PACK_BY_VERTICE);
		_VBOQuadFS->setElementDescription(0, YVbo::Element(3)); //Sommet
		_VBOQuadFS->createVboCpu();
		_VBOQuadFS->setElementValue(0, 0, -1, -1, 0);
		_VBOQuadFS->setElementValue(0, 1, 1, -1, 0);
		_VBOQuadFS->setElementValue(0, 2, -1, 1, 0);
		_VBOQuadFS->setElementValue(0, 3, -1, 1, 0);
		_VBOQuadFS->setElementValue(0, 4, 1, -1, 0);
		_VBOQuadFS->setElementValue(0, 5, 1, 1, 0);
		_VBOQuadFS->createVboGpu();
		_VBOQuadFS->deleteVboCpu();

		TimerGPU = timerGPU;
	}

	void resize(int screen_width, int screen_height)
	{
		ScreenWidth = screen_width;
		ScreenHeight = screen_height;
	}

	void setBackgroundColor(YColor color)
	{
		BackGroundColor = color;
	}

	void setRenderObjectFun(void(*fun)(void))
	{
		_RenderObjectsFun = fun;
	}

	void setRender2DFun(void(*fun)(void))
	{
		_Render2DFun = fun;
	}

	void drawFullScreenQuad() {

		glMatrixMode(GL_PROJECTION); // passe en mode matrice de projection
		glLoadIdentity(); // Réinitialisation

		glMatrixMode(GL_MODELVIEW); // on repasse en mode matrice modèle
		glLoadIdentity(); // Réinitialisation

		_VBOQuadFS->render();
	}

	void render(float elapsed)
	{
		if (TakeScreenShot) {
			YFbo::saveFbToFile(NameScreenShot.c_str(), this->ScreenWidth, this->ScreenHeight);
			YLog::log(YLog::ENGINE_INFO, ("Screen shot " + NameScreenShot + " saved").c_str());
			TakeScreenShot = false;
		}

		//On clear
		glClearColor(BackGroundColor.R, BackGroundColor.V, BackGroundColor.B, BackGroundColor.A);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		NbVBOFacesRendered = 0;

		//On set les globales
		glEnable(GL_DEPTH_TEST);

		//On efface les matrices
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Camera->update(elapsed);
		Camera->look(); //Initialise la matrice MV à V

		//Rendu de la scène
		if (_RenderObjectsFun != NULL)
			(*_RenderObjectsFun)();

		//Rendu 2D (UI et autres)

		//Mode 2D
		glUseProgram(0);
		glColor3f(1.0f, 1.0f, 1.0f);
		glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION); // passe en mode matrice de projection
		glLoadIdentity(); // Réinitialisation
		glOrtho(0, ScreenWidth, ScreenHeight, 0, -1.0f, 2.0f); //Full screen ortho avec taille en pixels

		glMatrixMode(GL_MODELVIEW); // on repasse en mode matrice modèle
		glLoadIdentity(); // Réinitialisation

		//On fait le rendu 2D sur l'écran
		if (_Render2DFun != NULL)
			(*_Render2DFun)();

		//Fini
		if (TimerGPU) TimerGPU->startAccumPeriod();
		glutSwapBuffers();
		glutPostRedisplay();
		if (TimerGPU) TimerGPU->endAccumPeriod();

		
	}

	//Faire un screen shot
	void screenShot(const char * name) {
		TakeScreenShot = true;
		NameScreenShot = string(name);
	}

	//GESTION DES SHADERS

	/**
		* Permet de créer un programme de shaders, a activer quand on veut
		*/
	GLuint createProgram(const char * dossier)
	{
		string fileFragmentShader = string(dossier) + string("/fs.glsl");
		string fileVertexShader = string(dossier) + string("/vs.glsl");
		string fileGeometryShader = string(dossier) + string("/gs.glsl");

		GLuint fs = loadShader(GL_FRAGMENT_SHADER, fileFragmentShader.c_str());
		GLuint vs = loadShader(GL_VERTEX_SHADER, fileVertexShader.c_str());
		GLuint gs = loadShader(GL_GEOMETRY_SHADER, fileGeometryShader.c_str());

		if (fs > 0 || vs > 0 || gs > 0) {
			GLuint prog = glCreateProgram();
			if (fs > 0) {
				glAttachShader(prog, fs);
				checkGlError("glAttachShader(prog, fs);");
			}

			if (vs > 0) {
				glAttachShader(prog, vs);
				checkGlError("glAttachShader(prog, vs);");
			}

			if (gs > 0) {
				glAttachShader(prog, gs);
				checkGlError("glAttachShader(prog, gs);");
			}


			glLinkProgram(prog);
			checkGlError("glLinkProgram(prog);");

			GLint isLinked = 0;
			glGetProgramiv(prog, GL_LINK_STATUS, &isLinked);
			if (isLinked == GL_FALSE) {
				GLint maxLength = 0;
				glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &maxLength);

				//The maxLength includes the NULL character
				char * infoLog = new char[maxLength];
				memset(infoLog, 0, maxLength * sizeof(char));


				glGetProgramInfoLog(prog, maxLength, &maxLength, infoLog);

				//The program is useless now. So delete it.
				glDeleteProgram(prog);

				//Provide the infolog in whatever manner you deem best.
				std::string error = "Unable to link";
				if (fs != 0)
					error += " fs[" + fileFragmentShader + "]";
				if (vs != 0)
					error += " vs[" + fileVertexShader + "]";
				if (gs != 0)
					error += " gs[" + fileGeometryShader + "]";

				error += " because " + toString(infoLog);
				YLog::log(YLog::ENGINE_ERROR, error.c_str());

				SAFEDELETE(infoLog);

				return 0;
			}

			return prog;
		}

		return 0;
	}


	void updateMatricesFromOgl() {
		float matMvTab[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, matMvTab);
		memcpy(MatMV.Mat.t, matMvTab, 16 * sizeof(float));
		MatMV.transpose();

		float matProjTab[16];
		glGetFloatv(GL_PROJECTION_MATRIX, matProjTab);
		memcpy(MatP.Mat.t, matProjTab, 16 * sizeof(float));
		MatP.transpose();

		MatMVP = MatP;
		MatMVP *= MatMV;

		MatV.createViewMatrix(Camera->Position, Camera->LookAt, Camera->UpVec);

		MatIV = MatV;
		MatIV.invert();

		MatM = MatIV;
		MatM *= MatMV;

		MatIM = MatM;
		MatIM.invert();

		MatNorm = MatM;
		MatNorm.invert();
		MatNorm.transpose();

		MatIP = MatP;
		MatIP.invert();
	}

	void sendTimeToShader(float elapsed, int prog) {
		if (prog == CURRENT_SHADER)
			glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		if (prog == 0)
			return;

		GLuint var = glGetUniformLocation(prog, "elapsed");
		glUniform1f(var, (float)(elapsed));
	}

	void sendScreenSizeToShader(int prog) {
		if (prog == CURRENT_SHADER)
			glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		if (prog == 0)
			return;

		GLuint var = glGetUniformLocation(prog, "screen_width");
		glUniform1f(var, (float)(ScreenWidth));

		var = glGetUniformLocation(prog, "screen_height");
		glUniform1f(var, (float)(ScreenHeight));
	}

	void sendNearFarToShader(int prog) {
		if (prog == CURRENT_SHADER)
			glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		if (prog == 0)
			return;

		GLuint var = glGetUniformLocation(prog, "near_far");
		glUniform2f(var, (float)(Camera->Near), (float)(Camera->Far));
	}

	void sendMatricesToShader(int prog)
	{
		checkGlError("start sendToShader");

		if (prog == CURRENT_SHADER)
			glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		if (prog == 0)
			return;

		GLuint mvp = glGetUniformLocation(prog, "mvp");
		glUniformMatrix4fv(mvp, 1, true, MatMVP.Mat.t);

		GLuint mv = glGetUniformLocation(prog, "mv");
		glUniformMatrix4fv(mv, 1, true, MatMV.Mat.t);

		GLuint m = glGetUniformLocation(prog, "m");
		glUniformMatrix4fv(m, 1, true, MatM.Mat.t);

		GLuint v = glGetUniformLocation(prog, "v");
		glUniformMatrix4fv(v, 1, true, MatV.Mat.t);

		GLuint p = glGetUniformLocation(prog, "p");
		glUniformMatrix4fv(p, 1, true, MatP.Mat.t);

		GLuint im = glGetUniformLocation(prog, "im");
		glUniformMatrix4fv(im, 1, true, MatIM.Mat.t);

		GLuint iv = glGetUniformLocation(prog, "iv");
		glUniformMatrix4fv(iv, 1, true, MatIV.Mat.t);

		GLuint ip = glGetUniformLocation(prog, "ip");
		glUniformMatrix4fv(ip, 1, true, MatIP.Mat.t);

		GLuint nmat = glGetUniformLocation(prog, "nmat");
		glUniformMatrix4fv(nmat, 1, true, MatNorm.Mat.t);

		GLuint var = glGetUniformLocation(prog, "screen_width");
		glUniform1f(var, (float)(ScreenWidth));

		var = glGetUniformLocation(prog, "screen_height");
		glUniform1f(var, (float)(ScreenHeight));

		checkGlError("Fin sendToShader");
	}

	//Donner z = 1 pour etre au fond du buffer de profondeur
	void unProjectMousePos(int mouseX, int mouseY, float mouseZ, double * posX, double * posY, double * posZ)
	{
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ;

		//Mode rendu du monde car sinon elle sont en mode rendu du quad de post process
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		this->Camera->look();

		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glGetIntegerv(GL_VIEWPORT, viewport);

		winX = (float)mouseX;
		winY = (float)viewport[3] - (float)mouseY;
		winZ = mouseZ;

		gluUnProject(winX, winY, winZ, modelview, projection, viewport, posX, posY, posZ);
	}

	static void checkGlError(const char * call)
	{
		GLenum error = glGetError();

		if (error != 0) {
			switch (error) {
			case GL_INVALID_ENUM: YLog::log(YLog::ENGINE_ERROR, ("Opengl error (GL_INVALID_ENUM) for call " + toString(call)).c_str()); break;
			case GL_INVALID_OPERATION: YLog::log(YLog::ENGINE_ERROR, ("Opengl error (GL_INVALID_OPERATION) for call " + toString(call)).c_str()); break;
			case GL_STACK_OVERFLOW: YLog::log(YLog::ENGINE_ERROR, ("Opengl error (GL_STACK_OVERFLOW) for call " + toString(call)).c_str()); break;
			case GL_STACK_UNDERFLOW: YLog::log(YLog::ENGINE_ERROR, ("Opengl error (GL_STACK_UNDERFLOW) for call " + toString(call)).c_str()); break;
			case GL_OUT_OF_MEMORY: YLog::log(YLog::ENGINE_ERROR, ("Opengl error (GL_OUT_OF_MEMORY) for call " + toString(call)).c_str()); break;
			case GL_TABLE_TOO_LARGE: YLog::log(YLog::ENGINE_ERROR, ("Opengl error (GL_TABLE_TOO_LARGE) for call " + toString(call)).c_str()); break;
			default: YLog::log(YLog::ENGINE_ERROR, ("Unknown Opengl error for call " + toString(call)).c_str()); break;
			}
			Sleep(5000);
		}
	}



private:

	char* loadSource(const char *filename)
	{
		char *src = NULL;   /* code source de notre shader */
		FILE *fp = NULL;    /* fichier */
		long size;          /* taille du fichier */
		long i;             /* compteur */


							/* on ouvre le fichier */
		fopen_s(&fp, filename, "r");
		/* on verifie si l'ouverture a echoue */
		if (fp == NULL) {
			YLog::log(YLog::ENGINE_WARNING, (std::string("Unable to load shader file ") + std::string(filename)).c_str());
			return NULL;
		}

		/* on recupere la longueur du fichier */
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);

		/* on se replace au debut du fichier */
		rewind(fp);

		/* on alloue de la memoire pour y placer notre code source */
		src = (char*)malloc(size + 1); /* +1 pour le caractere de fin de chaine '\0' */
		if (src == NULL) {
			fclose(fp);
			YLog::log(YLog::ENGINE_WARNING, "Unable to allocate memory for shader file before compilation");
			return NULL;
		}

		/* lecture du fichier */
		for (i = 0; i < size && !feof(fp); i++)
			src[i] = fgetc(fp);

		/* on place le dernier caractere a '\0' */
		src[i] = '\0';

		//sous windows, EOF a virer
		if (src[i - 1] == EOF)
			src[i - 1] = '\0';

		fclose(fp);

		return src;
	}

	GLuint loadShader(GLenum type, const char *filename)
	{
		GLuint shader = 0;
		GLsizei logsize = 0;
		GLint compile_status = GL_TRUE;
		char *log = NULL;
		char *src = NULL;

		FILE * ftest;
		if (fopen_s(&ftest, filename, "r") == 0)
			fclose(ftest);
		else
			return 0;

		// creation d'un shader de sommet
		shader = glCreateShader(type);
		if (shader == 0) {
			YLog::log(YLog::ENGINE_ERROR, (toString("Unable to create shader ") + filename).c_str());
			return 0;
		}

		// chargement du code source
		src = loadSource(filename);
		if (src == NULL) {
			// theoriquement, la fonction LoadSource a deja affiche un message
			// d'erreur, nous nous contenterons de supprimer notre shader
			// et de retourner 0

			glDeleteShader(shader);
			return 0;
		}

		// assignation du code source
		glShaderSource(shader, 1, (const GLchar**)&src, NULL);

		// compilation du shader
		glCompileShader(shader);

		// liberation de la memoire du code source
		free(src);
		src = NULL;

		//verification du succes de la compilation
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);

		log = (char*)malloc(logsize + 1);
		if (log == NULL) {
			YLog::log(YLog::ENGINE_ERROR, "Unable to allocate memory for shader compilation log");
			return 0;
		}
		memset(log, '\0', logsize + 1);

		glGetShaderInfoLog(shader, logsize, &logsize, log);

		if (compile_status != GL_TRUE) {

			YLog::log(YLog::ENGINE_ERROR, ("Unable to compile shader " + toString(filename) + " : " + toString(log)).c_str());
			free(log);
			glDeleteShader(shader);

			return 0;
		} else {
			YLog::log(YLog::ENGINE_INFO, ("Shader " + toString(filename) + " compilation ok").c_str());
			if (toString(log).length() > 0)
				YLog::log(YLog::ENGINE_INFO, ("Compile res " + toString(filename) + " : " + toString(log)).c_str());
			free(log);
		}

		return shader;
	}

};

#endif 