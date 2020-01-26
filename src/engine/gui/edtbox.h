#pragma once

#include "engine/gui/panel.h"

#define EDIT_BOX_HEIGHT_BASE 20

class GUIEdtBox : public GUIPanel
{
	public:
		std::string Text;
		YColor ColorSel;
		int FontNum;

	private:
		bool _StartSelection;
		uint16 _IdStartSelection;
		uint16 _IdEndSelection;
		uint16 _OffsetAffichage;
		bool _IsPassword;
		std::string _TextToDraw;
		

	public:
		GUIEdtBox() : GUIPanel()
		{
			this->X = 10;
			this->Y = 10;
			this->Width = 100;
			this->Height = EDIT_BOX_HEIGHT_BASE;
			ColorSel.R = 0.0;
			ColorSel.V = 1.0;
			ColorSel.B = 0.0;
			_StartSelection = false;
			_IdStartSelection = 0;
			_IdEndSelection = 0;
			_IsPassword = false;
			FontNum = 0;
		}

		void resetSelection(void)
		{
			_IdStartSelection = _IdEndSelection = 0;
		}

		void setPassword(bool isPwd)
		{
			_IsPassword = isPwd;
		}

		void setText(std::string & text)
		{
			Text = text;
			_TextToDraw = text;
		}

		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				GUIPanel::render(zorder);
				
				//Draw String
				TextEngine->SelectFont(FontNum);
				uint16 up,down;
				TextEngine->fontHeight(up,down);
				sint16 lenAff = (Width-5) / TextEngine->fontWidth();
				glColor3f(ColorBorder.R,ColorBorder.V,ColorBorder.B);
				glRasterPos2i(X+5,Y + (Height+up)/2);

				if(_IdStartSelection < _OffsetAffichage)
					_OffsetAffichage = _IdStartSelection;
				if(_IdStartSelection > _OffsetAffichage+lenAff-1)
					_OffsetAffichage = _IdStartSelection-lenAff+1;
			
				TextEngine->glPrint(lenAff,_TextToDraw.c_str()+_OffsetAffichage);	

				if(_HasFocus) 
				{
					glRasterPos2i(X+5+(_IdEndSelection-_OffsetAffichage)*TextEngine->fontWidth(),Y + (Height+up)/2);
					TextEngine->glPrint(1,"_");
				}

				//Draw sel string
				if(_IdStartSelection != _IdEndSelection)
				{
					glColor3f(ColorSel.R,ColorSel.V,ColorSel.B);
					
					if((sint16) _TextToDraw.length() > lenAff-1)
					{
						glRasterPos2i(X+5+(_IdStartSelection-_OffsetAffichage)*TextEngine->fontWidth(),Y + (Height+up)/2);
						TextEngine->glPrint(_IdEndSelection - _IdStartSelection,_TextToDraw.c_str()+_IdStartSelection);
					}
					else
					{
						glRasterPos2i(X+5+_IdStartSelection*TextEngine->fontWidth(),Y + (Height+up)/2);
						TextEngine->glPrint(_IdEndSelection - _IdStartSelection,_TextToDraw.c_str()+_IdStartSelection);
					}
				}		
			}
		}

		sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder, bool focusAvailable, uint32 elapsed)
		{
			sint8 mouseForMe = 0;
			
			if(ZOrder == zorder)
			{
				GUIPanel::mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed);

				if(MouseOn && focusAvailable)
				{
					mouseForMe = 1;

					if(click & GUI_MLBUTTON)
					{
						_HasFocus = true;
						uint16 index = (uint16) ( (((x-X) - (TextEngine->fontWidth()/2)) / TextEngine->fontWidth()) + _OffsetAffichage );
						if(index > Text.length())
						{
							index = (uint16)Text.length();
						}

						if(_StartSelection == false)
						{
							_StartSelection = true;
							_IdStartSelection = index;
							_IdEndSelection = index;
						}
						else
						{
							if (index > _IdStartSelection)
								_IdEndSelection = index;
						}
					}
				}
				else
				{
					_StartSelection = false;
					if(click & GUI_MLBUTTON)
					{
						_HasFocus = false;
						_IdEndSelection = _IdStartSelection;
					}
				}

				if(!(click & GUI_MLBUTTON))
				{
					_StartSelection = false;
				}
			}
			return mouseForMe;
		}

		sint8 keyCallback(char car, bool * keys, uint32 elapsed)
		{
			if(_HasFocus)
			{
				if ((unsigned char)car >= 0x20 || keys[GUI_KEY_BACK] || keys[GUI_KEY_DELETE])
				{
					if(_IdStartSelection != _IdEndSelection)
					{
						Text = Text.erase(_IdStartSelection,_IdEndSelection-_IdStartSelection);
						_IdEndSelection = _IdStartSelection;
						keys[GUI_KEY_BACK] = false;
						keys[GUI_KEY_DELETE] = false;
					}
				}

				std::string begin = Text.substr(0,_IdStartSelection);
				std::string end = Text.substr(_IdStartSelection,Text.length() - _IdStartSelection);
				
				if ((unsigned char)car >= 0x20 && car != 0x7f)
				{
					Text = begin+car+end;
					_IdStartSelection++;
					_IdEndSelection++;
				}

				if(keys[GUI_KEY_BACK])
				{
					if(begin.length())
					{
						Text = begin.substr(0,begin.length()-1) + end;
						_IdStartSelection--;
						_IdEndSelection--;
					}
					keys[GUI_KEY_BACK] = false;
				}

				if(keys[GUI_KEY_DELETE])
				{
					if(end.length())
					{
						Text = begin + end.substr(1,end.length());
					}
					keys[GUI_KEY_DELETE] = false;
				}
				
				if(_IsPassword)
				{
					_TextToDraw.erase();
					for(unsigned int i=0;i<Text.length();i++)
						_TextToDraw += "*";
				}
				else
					_TextToDraw = Text;

				return 1;
			}
			return 0;
		}

		sint8 specialKeyCallback(char car,bool * keys,uint32 elapsed)
		{
			if(_HasFocus)
			{
				if(keys[GLUT_KEY_RIGHT] && _IdStartSelection == _IdEndSelection)
				{
					if (_IdStartSelection >= 0 && _IdStartSelection < Text.length())
					{
						_IdStartSelection++;
						_IdEndSelection++;
						keys[GLUT_KEY_RIGHT] = false;
					}

				}

				if(keys[GLUT_KEY_LEFT] && _IdStartSelection == _IdEndSelection)
				{
					if (_IdStartSelection > 0)
					{
						_IdStartSelection--;
						_IdEndSelection--;
						keys[GLUT_KEY_LEFT] = false;
					}

				}

				return 1;
			}
			return 0;
		}

	private:

};