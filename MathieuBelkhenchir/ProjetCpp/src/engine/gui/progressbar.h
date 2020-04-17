#pragma once

#include "engine/gui/panel.h"

#define GUI_PBAR_DEFAULT_HEIGHT 20

class GUIPBar : public GUIPanel
{
	public:
		double Max;
		double Min;
		double Value;
		bool Border;

	private:
		GUIPanel * _Ascenceur;
		class Bar
		{
			public:
				float Value;
				YColor YColor;
				sint32 Width;
		};
		std::vector<Bar> _Bars;
		GUIPanel * _BarDrawer;

	private:

		void enforceMaxMin(void)
		{
			if(Value < Min)
				setValue(Min);
			if(Value > Max)
				setValue(Max);
		}


	public:
		GUIPBar() : GUIPanel()
		{
			Border = true;
			Width = 250;
			Height = GUI_PBAR_DEFAULT_HEIGHT;
			Value = 0;
						
			_Ascenceur = new GUIPanel();
			_Ascenceur->Width = 0;
			_Ascenceur->Height = GUI_PBAR_DEFAULT_HEIGHT;
			_Ascenceur->FondPlein = true;

			_BarDrawer =  new GUIPanel();
			_BarDrawer->Width = 1;
			_BarDrawer->Height = GUI_PBAR_DEFAULT_HEIGHT;
			_BarDrawer->FondPlein = true;

			setSize(Width,Height);
			setPos(X,Y);
			setMaxMin(1.0,0.0);
			setValue(0.0);

			_Ascenceur->ColorFond.R = 0.8f;
			_Ascenceur->ColorFond.V = 0.8f;
			_Ascenceur->ColorFond.B = 0.8f;

			
		}

		void addBar(float val, YColor color, sint32 width = 1)
		{
			Bar bar;
			_Bars.push_back(bar);
			_Bars[_Bars.size() - 1].YColor = color;
			_Bars[_Bars.size()-1].Value = val;
			_Bars[_Bars.size()-1].Width = width;
		}

		void clearBars(void)
		{
			_Bars.clear();
		}

		void setPos(uint16 x, uint16 y)
		{
			X = x;
			Y = y;

			setAscenceurPos(Value);
		}
		
		void setSize(uint16 width, uint16 height)
		{
			Width = width;
			Height = height;
			_Ascenceur->Height = height;
			_BarDrawer->Height = height;
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
			if(max < min)
			{
				Min = max;
				Max = min;
			}
			enforceMaxMin();
		}

		void setColorFond(YColor & color)
		{
			_Ascenceur->ColorFond = color;
		}

				
		sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder, bool focusAvailable, uint32 elapsed)
		{
			return GUIPanel::mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed);
		}

		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				if(Border)
					GUIPanel::render(zorder);

				_Ascenceur->render(ZOrder);

				for(unsigned int i=0;i<_Bars.size();i++)
				{
					_BarDrawer->X = X + (sint32)getWidthOnBar(_Bars[i].Value);
					_BarDrawer->Y = Y;
					_BarDrawer->ColorBorder = _Bars[i].YColor;
					_BarDrawer->ColorFond = _Bars[i].YColor;
					_BarDrawer->Width = _Bars[i].Width;
					_BarDrawer->render(ZOrder);
				}
			}
		}

		private:
			void setAscenceurPos(double Value)
			{				
				_Ascenceur->X = X;
				_Ascenceur->Y = Y;
				_Ascenceur->Width = (uint32) ( getWidthOnBar(Value) );
			}

			double getWidthOnBar(double value)
			{
				if(value < Min)
					value = Min;
				if(value > Max)
					value = Max;

				value -= Min;
				value /= Max-Min;

				return value * Width;
			}
};