#ifndef __SCREEN_MANAGER__
#define __SCREEN_MANAGER__

#include "engine/gui/screen.h"

class GUIScreenManager
{
	public:
	
		GUIScreen * _ActiveScreen;
		bool _Keys[256]; 
		bool _SpecialKeys[256]; 

		GUIScreenManager()
		{
			_ActiveScreen = NULL;
		}

		bool hasActiveScreen(void)
		{
			return _ActiveScreen != NULL;
		}

		void render(void)
		{
			if(_ActiveScreen)
				_ActiveScreen->render();
		}

		void setActiveScreen(GUIScreen * screen)
		{
			_ActiveScreen = screen;
		}

		bool mouseCallback(int x, int y, uint32 click, sint32 wheel, uint32 elapsed)
		{
			if(_ActiveScreen)
				return _ActiveScreen->mouseCallback(x,y,click,wheel,elapsed);
			return false;
		}

		sint8 keyCallback(char car, bool down, uint32 elapsed)
		{
			_Keys[car] = down;
			if(_ActiveScreen && down)
				return _ActiveScreen->keyCallback(car,_Keys,elapsed);
			return 0;
		}

		sint8 specialKeyCallback(int car,bool down, uint32 elapsed)
		{
			_SpecialKeys[car] = down;
			if(_ActiveScreen && down)
				return _ActiveScreen->specialKeyCallback(car,_SpecialKeys,elapsed);
			return 0;
		}

		void update(float elapsed)
		{
			if(_ActiveScreen)
				_ActiveScreen->update(elapsed);
		}
};


#endif