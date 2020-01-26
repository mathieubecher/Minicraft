#pragma once

#include "engine/gui/panel.h"

#include <vector>

class GUILstBox : public GUIPanel
{
	public:
		std::vector<std::string> Texts;
		YColor ColorSel;
		uint32 LastClicked;
		uint32 LastChanged;
		bool AlphaEffect;
		bool ShowScrollBar;
		bool CenterSelected;
		
					
	private:
		sint16 _IdSelection;
		uint16 _ElementHeight;
		sint16 _OffsetAffichage;
		GUIPanel * _Ascenceur;
		uint16 _NbMaxElements;
		int _FontNum;
		
		
	public:
		GUILstBox() : GUIPanel()
		{
			setFontNum(0);

			_Ascenceur = new GUIPanel();
			_Ascenceur->Visible = false;
			_Ascenceur->FondPlein = true;
						
			setMaxElements(5);
			this->X = 10;
			this->Y = 10;
			this->Width = 100;
			ColorSel.R = 0.0;
			ColorSel.V = 1.0;
			ColorSel.B = 0.0;
			_IdSelection = -1;
			_OffsetAffichage = 0;
			LastClicked = 0;
			LastChanged = 0;
			ShowScrollBar = true;
			CenterSelected = false;

			AlphaEffect = false;
			
		}

		void setFontNum(int num)
		{
			_FontNum = num;
			uint16 up,down;
			TextEngine->SelectFont(_FontNum);
			TextEngine->fontHeight(up,down);
			_ElementHeight = (up+down+2);
		}

		uint16 getElementHeight(void)
		{
			return _ElementHeight;
		}

		uint16 getNbElements(void)
		{
			return (uint16)Texts.size();
		}

		uint16 getNbMaxElements(void)
		{
			return _NbMaxElements;
		}

		sint16 getSelIndex(void)
		{
			return _IdSelection;
		}

		void setSelIndex(uint16 index)
		{
			if (index < Texts.size())
				_IdSelection = index;
			if(CenterSelected)
				centerSelected();
		}

		void setZOrder(uint16 zorder)
		{
			ZOrder = zorder;
			_Ascenceur->ZOrder = zorder;
		}

		sint8 setMaxElements(uint16 nbMaxElements)
		{
			uint16 up,down;
			TextEngine->SelectFont(_FontNum);
			TextEngine->fontHeight(up,down);
			_NbMaxElements = nbMaxElements;
			Height = _NbMaxElements * _ElementHeight + down+2;
			return 0;
		}

		//détermine le nombre d'éléments max a partir d'une hauteur
		sint8 setMaxElementsFromHeight(sint32 height)
		{
			uint16 up,down;
			TextEngine->SelectFont(_FontNum);
			TextEngine->fontHeight(up,down);
			_NbMaxElements = (height - down - 2) / _ElementHeight;
			Height = _NbMaxElements * _ElementHeight + down+2;
			return 0;
		}

		sint8 addElement(std::string & element)
		{
			Texts.push_back(element);
			if(Texts.size() > _NbMaxElements && ShowScrollBar)
				showAscenceur(true);
			return 0;
		}

		void clear(void)
		{
			Texts.clear();
			showAscenceur(false);
		}

		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				GUIPanel::render(zorder);
				_Ascenceur->render(zorder);

				uint16 up,down;
				TextEngine->SelectFont(_FontNum);
				TextEngine->fontHeight(up,down);
				uint16 lenAff = (Width-5) / TextEngine->fontWidth();

				glColor3f(ColorBorder.R,ColorBorder.V,ColorBorder.B);

				//Draw Strings
				for(unsigned int i=0; i < _NbMaxElements;i++)
				{
					float alpha = 1.0f;

					if(AlphaEffect)
						alpha = 1.0f-(float)abs(i-_NbMaxElements/2.0f)/(( _NbMaxElements+1)/2.0f);

					if (i+_OffsetAffichage == _IdSelection)
						glColor4f(ColorSel.R,ColorSel.V,ColorSel.B,alpha);
					else
						glColor4f(ColorBorder.R,ColorBorder.V,ColorBorder.B,alpha);

					if(_OffsetAffichage + i >= 0 && _OffsetAffichage + i < Texts.size())
					{
						glRasterPos2i(X+5,Y + (_ElementHeight * (i+1)));
						TextEngine->glPrint(lenAff-1,Texts[i+_OffsetAffichage].c_str());
					}

					if (i+_OffsetAffichage == _IdSelection)
						glColor3f(ColorBorder.R,ColorBorder.V,ColorBorder.B);
				}		
			}
		}

		sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder, bool focusAvailable, uint32 elapsed)
		{
			sint8 mouseForMe = 0;
			if(Visible && zorder == ZOrder)
			{
				GUIPanel::mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed);
				 
				if(MouseOn && focusAvailable)
				{
					mouseForMe = 1;

					if(click & GUI_MLBUTTON)
					{
						uint16 up,down;
						TextEngine->SelectFont(_FontNum);
						TextEngine->fontHeight(up,down);

						_HasFocus = true;
						//_IdSelection = (((y-Y) - (_ElementHeight/2)+down) / _ElementHeight) + _OffsetAffichage;
						_IdSelection = (sint16) ( ((y-Y) / _ElementHeight) + _OffsetAffichage );
						
						if (_IdSelection >= (sint16) Texts.size())
							_IdSelection = (sint16) Texts.size() - 1;
						if (_IdSelection < 0)
							_IdSelection = 0;

						if(CenterSelected)
							centerSelected();

						LastClicked = GetTickCount();
					}

					if(wheel>0)
						moveSelection(true);
					if(wheel<0)
						moveSelection(false);
				}
				else
				{				
					if(click & MK_LBUTTON)
					{
						_HasFocus = false;
					}
				}
			}

			return mouseForMe;
		}

		sint8 specialKeyCallback(char car,bool * keys, uint32 elapsed)
		{
			if(Visible)
			{
				if(_HasFocus)
				{
					if(keys[GLUT_KEY_DOWN])
					{
						moveSelection(false);
						keys[GLUT_KEY_DOWN] = false;
					}

					if(keys[GLUT_KEY_UP])
					{
						moveSelection(true);
						keys[GLUT_KEY_UP] = false;
					}
				}
			}
			return 0;
		}

		void centerSelected(void)
		{
			_OffsetAffichage = _IdSelection -  (_NbMaxElements/2);
		}

	private:
		void showAscenceur(bool show)
		{
			_Ascenceur->Width = 8;
			_Ascenceur->Height = 8;
			
			if(show && !_Ascenceur->Visible)
				Width -= _Ascenceur->Width + 5;
			if(!show && _Ascenceur->Visible)
				Width += _Ascenceur->Width + 5;

			_Ascenceur->X = X + Width + 5;
			_Ascenceur->Y = Y + _OffsetAffichage;

			_Ascenceur->Visible = show;		
		}

		void moveSelection(bool up)
		{
			LastChanged = GetTickCount();

			if(up)
			{
				if (_IdSelection > 0)
				{	_IdSelection--;
					if (_IdSelection < _OffsetAffichage)
					{
						_OffsetAffichage--;
						_Ascenceur->Y = (uint32) ( Y+(Height - _Ascenceur->Height) * (float)((float)_OffsetAffichage/(float)(Texts.size()-_NbMaxElements)) );
					}
				}
			}
			else
			{
				if (_IdSelection >= 0 && _IdSelection + 1  < (sint16) Texts.size())
				{
					_IdSelection++;
					if (_IdSelection - _OffsetAffichage >= _NbMaxElements)
					{
						_OffsetAffichage++;
						_Ascenceur->Y = (uint32) ( Y+(Height - _Ascenceur->Height) * (float)((float)_OffsetAffichage/(float)(Texts.size()-_NbMaxElements)) );
					}
				}
			}

			if(CenterSelected)
				centerSelected();
		}

};