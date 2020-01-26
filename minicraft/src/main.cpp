//Includes application
#include <conio.h>
#include <vector>
#include <string>
#include <windows.h>

#include "engine_minicraft.h"
#include "external/gl/wglew.h"
/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main(int argc, char* argv[])
{ 
	YEngine * engine = MEngineMinicraft::getInstance();

	engine->initBase(argc,argv);

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)glutGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(1);

	glutMainLoop(); 

	return 0;
}

