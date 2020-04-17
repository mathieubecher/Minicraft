//Includes application
#include <conio.h>
#include <vector>
#include <string>
#include <windows.h>

#include "engine_test.h"

/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main(int argc, char* argv[])
{ 
	YEngine * engine = YEngineTest::getInstance();

	engine->initBase(argc,argv);

	glutMainLoop(); 

	return 0;
}

