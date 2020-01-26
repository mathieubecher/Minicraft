#pragma once

#include "engine/gui/image.h"

class GUILoading : public GUIImage
{
	private:
		float _Angle;
	public:
		GUILoading() : GUIImage()
		{
			_Angle = 0;
		}

		void update(float elapsed)
		{
			_Angle += elapsed * 360.0f;
		}

		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				glPushMatrix();
				glTranslatef(X,Y,0);
				glTranslatef(+Width/2.0f,+Height/2.0f,0);
				glRotatef(_Angle,0,0,1);
				glTranslatef(-Width/2.0f,-Height/2.0f,0);
				//On le fait direct pour pouvoir appliquer la rotation correctement
				if(_Tex)
					YTexManager::getInstance()->drawTex2D(0,0,1,1,0,0,*_Tex);
				glPopMatrix();
			}
		}
};