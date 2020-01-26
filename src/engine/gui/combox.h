#pragma once

#include "edtbox.h"
#include "bouton.h"
#include "lstbox.h"

class GUIComboBox : public GUIPanel
{
	public:
		GUIBouton * Bouton;
		GUIEdtBox * EdtBox;
		GUILstBox * LstBox;
		bool Border;

	private:
		bool _StateBtn;		
		uint32 _LastClickLstBox;

	public:
		GUIComboBox() : GUIPanel()
		{
			_StateBtn = false;
			Border = false;
			_LastClickLstBox = 0;

			EdtBox = new GUIEdtBox();
			LstBox = new GUILstBox();
			LstBox->ColorFond = YColor(0.0f,0.0f,0.0f,0.5f);

			setPos(10,10,100,100);
		}

		void setPos(uint16 x,uint16 y,uint16 width,uint16 nbElementsAff)
		{
			X = x;
			Y = y;
			Width = width;
			
			EdtBox->X = X;
			EdtBox->Y = Y;
			EdtBox->Width = Width;
			EdtBox->Height = EDIT_BOX_HEIGHT_BASE;

			LstBox->X = X;
			LstBox->Y = Y + EdtBox->Height + 5;
			LstBox->Width = Width;
			LstBox->setMaxElements(nbElementsAff);
			LstBox->FondPlein = true;
			LstBox->setZOrder(1);

			Height = EdtBox->Height + LstBox->Height + 5;
		}

		void render(uint16 zorder)
		{
			if(Border)
				GUIPanel::render(zorder);

			if(_LastClickLstBox < LstBox->LastClicked || _LastClickLstBox < LstBox->LastChanged)
			{
				_LastClickLstBox = LstBox->LastClicked > LstBox->LastChanged ? LstBox->LastClicked : LstBox->LastChanged;
				if(LstBox->getSelIndex() >= 0)
				{
					EdtBox->setText(LstBox->Texts[LstBox->getSelIndex()]);
					EdtBox->resetSelection();
				}
			}

			EdtBox->render(zorder);
			
			if(LstBox->Visible && !MouseOn)
				LstBox->Visible = false;
			
			if(!LstBox->Visible && EdtBox->hasFocus() && EdtBox->MouseOn)
				LstBox->Visible = true;

			LstBox->render(zorder);	
		}

		void setSelIndex(uint16 index)
		{
			LstBox->setSelIndex(index);
			if(LstBox->getSelIndex() >= 0)
			{
				EdtBox->setText(LstBox->Texts[LstBox->getSelIndex()]);
				EdtBox->resetSelection();
			}
		}

		sint16 getSelIndex(void)
		{
			return LstBox->getSelIndex();
		}

		sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder,bool focusAvailable, uint32 elapsed)
		{
			sint8 mouseForMe = 0;

			GUIPanel::mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed);
			if(MouseOn && LstBox->Visible && zorder == LstBox->ZOrder)
				mouseForMe = 1;
			
			if(EdtBox->mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed))
				mouseForMe = 1;
			if(LstBox->mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed))
			{
				mouseForMe = 1;
			}
		
			return mouseForMe;
		}

		sint8 keyCallback(char car, bool * keys, uint32 elapsed)
		{
			EdtBox->keyCallback(car,keys,elapsed);
			LstBox->keyCallback(car,keys,elapsed);
			return 0;
		}

		sint8 specialKeyCallback(char car, bool * keys, uint32 elapsed)
		{
			EdtBox->specialKeyCallback(car,keys,elapsed);
			LstBox->specialKeyCallback(car,keys,elapsed);
			return 0;
		}

	private:

};