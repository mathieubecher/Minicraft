#pragma once

#include "engine/gui/panel.h"


#define GUI_SLIDER_DEFAULT_HEIGHT 20


class GUISlider : public GUIPanel
{
	public:
		double Max;
		double Min;
		double Value;
		bool Border;
		double XFactor;

	private:
		GUIPanel * _Ascenceur;
		GUIPanel * _Track;

	private:

		void enforceMaxMin(void)
		{
			if(Value < Min)
				setValue(Min);
			if(Value > Max)
				setValue(Max);
		}


	public:
		GUISlider() : GUIPanel()
		{
			Border = false;
			Width = 250;
			Height = GUI_SLIDER_DEFAULT_HEIGHT;
			Value = 0;
						
			_Ascenceur = new GUIPanel();
			_Ascenceur->Width = 10;
			_Ascenceur->Height = 20;
			_Ascenceur->FondPlein = true;

			_Track = new GUIPanel();
			_Track->FondPlein = true;
			
			setSize(Width,Height);
			setPos(X,Y);
			setMaxMin(1.0,0.0);
			setValue(0.0);
		}

		void setPos(uint16 x, uint16 y)
		{
			X = x;
			Y = y;

			_Track->X = X+_Ascenceur->Width/2;
			_Track->Y = Y+_Ascenceur->Height/2 - _Track->Height/2;

			setAscenceurPos(Value);
		}
		
		void setSize(uint16 width, uint16 height)
		{
			Width = width;
			Height = height;
			
			_Track->Width = Width - _Ascenceur->Width;
			_Track->Height = 5;

			_Ascenceur->Height = height;
		}

		void setValue(double value)
		{
			Value = value;
			enforceMaxMin();
			setAscenceurPos(value);
		}

		void setMaxMin(double max, double min)
		{
			Max = max;
			Min = min;
			double relMax = Max-Min;
			XFactor = _Track->Width / relMax;
			enforceMaxMin();
		}
				
		sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder, bool focusAvailable, uint32 elapsed)
		{
			sint8 mouseForMe = 0;

			if(zorder == ZOrder)
			{
				GUIPanel::mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed);

				if(MouseOn && click & GUI_MLBUTTON)
				{
					double val = x;
					val -= X+_Ascenceur->Width/2;
					val /= XFactor;
					val += Min;
					setValue(val);
					mouseForMe = 1;
				}
			}

			return mouseForMe;
		}
		

		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				if(Border)
					GUIPanel::render(zorder);

				_Track->render(ZOrder);
				_Ascenceur->render(ZOrder);
			}
		}

		private:
			void setAscenceurPos(double Value)
			{
				if(Value < Min)
					Value = Min;
				if(Value > Max)
					Value = Max;

				Value -= Min;
				double pos = XFactor * Value;
				_Ascenceur->X = (uint32) ( _Track->X+pos-_Ascenceur->Width/2 );
				_Ascenceur->Y = Y;
			}

};