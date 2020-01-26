#pragma once


#include "engine/gui/panel.h"

#define LABEL_HEIGHT_BASE 20


class GUILabel : public GUIPanel
{
	public:
		std::string Text;
		bool Centering;
		bool Border;
		int FontNum;

	public:
		GUILabel() : GUIPanel()
		{
			Text = std::string("Sans label");
			Centering = false;
			Border = false;
			Height = LABEL_HEIGHT_BASE;		
			FontNum = 0;
		}

		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				
				if(Border)
					GUIPanel::render(zorder);
				
				//Draw String
				uint16 up,down;
				TextEngine->SelectFont(FontNum);
				TextEngine->fontHeight(up,down);
				uint16 lenAff = (uint16)Text.length() * TextEngine->fontWidth();
				glColor3f(ColorBorder.R,ColorBorder.V,ColorBorder.B);

				if(Centering)
					glRasterPos2i(X+(Width-lenAff)/2,Y + (Height+up)/2);
				else
					glRasterPos2i(X,Y + (Height+up)/2);

				TextEngine->glPrint((uint16)Text.length(),Text.c_str());				
			}
		}
}; 