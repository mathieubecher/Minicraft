#pragma once

#include "engine/gui/panel.h"

#define BOUTON_HEIGHT_BASE 20

class GUIBouton;

typedef void (*FCT_CALL_BOUTON) (GUIBouton * bouton);

class GUIBouton : public GUIPanel
{
	public:
		std::string Titre;

	private:
		bool _ClickProcessed;
		bool _Validating;
		bool _Armed; //Pour ne pas cliquer par inadvertance, il faut d'abord passer sur le bouton sans LBUTTONDOW-N appuyé pour l'armer
		FCT_CALL_BOUTON _OnClick;

	public:
		GUIBouton() : GUIPanel()
		{
			Titre = std::string("Sans titre");
			this->X = 10;
			this->Y = 10;
			this->Width = 100;
			this->Height = BOUTON_HEIGHT_BASE;

			_ClickProcessed = false;
			_Validating = false;
			_Armed = false;
			_OnClick = NULL;
		}

		void setOnClick(FCT_CALL_BOUTON onClick)
		{
			_OnClick = onClick;
		}

		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				if(_Validating)
					drawSquareFilled(X,Y,Width,Height,ValidColor);
				else
					if(_HasFocus)
						drawSquareFilled(X,Y,Width,Height,FocusColor);

				GUIPanel::render(zorder);
				
				//Draw String
				uint16 up,down;
				TextEngine->fontHeight(up,down);
				uint16 lenAff = (uint16)(Titre.length() * TextEngine->fontWidth());
				glColor3f(ColorBorder.R,ColorBorder.V,ColorBorder.B);
				glRasterPos2i(X+(Width-lenAff)/2,Y + (Height+up)/2);
				TextEngine->glPrint((sint16)Titre.length(),Titre.c_str());				
			}
		}

		sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder, bool focusAvailable, uint32 elapsed)
		{
			sint8 mouseForMe = 0;

			if(zorder == ZOrder)
			{
				GUIPanel::mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed);

				_HasFocus = false;
				_Validating = false;
				if(MouseOn && focusAvailable)
				{
					mouseForMe = 1;
					_HasFocus = true;	

					_Validating = false;
					if(click & GUI_MLBUTTON)
					{
						if(_Armed)
						{
							_Validating = true;		
							if(!_ClickProcessed)
							{
								_ClickProcessed = true;
								_Armed = false;
								if(_OnClick)
									_OnClick(this);
							}
						}
					}
					else
					{
						_Armed = true;
						_ClickProcessed = false;
					}
				}
				else
				{
					_Armed = false;
				}
			}

			return mouseForMe;
		}
	private:

};