#pragma once

#include "engine/gui/panel.h"

class Courbe
{
	public :
		std::vector<YVec3f> Points;	
		std::vector<YVec3f> PointsScaled;
		YColor Color;

	public:
		Courbe(const YColor & color)
		{
			Color = color;
		}
};

typedef enum
{
	SLP_COURBE=0,
	SLP_BARRES
}TYPE_SLOPE;

class GUISlope : public GUIPanel
{
	public:
		std::vector<Courbe*> Courbes;	
		std::string Titre;
		float XFactor;
		float YFactor;
		float MargeX;
		float MargeY;
		float XMax;
		float XMin;
		float YMax;
		float YMin;
		bool ForceXMinMax;
		bool ForceYMinMax;
		TYPE_SLOPE Type;

	private:
		std::string _Coords;
		
	public:
		GUISlope() : GUIPanel()
		{
			Width = 250;
			Height = 150;
			MargeX = 60;
			MargeY = 60;
			XMax = 0;
			XMin = 0;
			YMax = 0;
			YMin = 0;
			Type = SLP_COURBE;
			ForceXMinMax = false;
			ForceYMinMax = false;
			
			//les axes
			addCourbe(YColor(ColorBorder.R,ColorBorder.V,ColorBorder.B,1.0));
			addCourbe(YColor(ColorBorder.R,ColorBorder.V,ColorBorder.B,1.0));
		}
		
		uint16 addCourbe(const YColor & color)
		{
			Courbe * courbe = new Courbe(color);
			Courbes.push_back(courbe);
			return (uint16)Courbes.size()-1;
		}

		void scaleSlopes(void)
		{
			if(Courbes[2]->Points.size() < 2)
				return;
			
			if(!ForceXMinMax)
			{
				XMax = 0;
				XMin = Courbes[2]->Points[0].X;
			}

			if(!ForceYMinMax)
			{
				YMax = 0;
				YMin = Courbes[2]->Points[0].Y;
			}
			
			for(unsigned int i=2;i<Courbes.size();i++)
			{
				for(unsigned int j=0;j<Courbes[i]->Points.size();j++)
				{
					if(!ForceXMinMax)
					{
						if(Courbes[i]->Points[j].X > XMax)
							XMax = Courbes[i]->Points[j].X;
						if(Courbes[i]->Points[j].X < XMin)
							XMin = Courbes[i]->Points[j].X;
					}

					if(!ForceYMinMax)
					{
						if(Courbes[i]->Points[j].Y > YMax)
							YMax = Courbes[i]->Points[j].Y;
						if(Courbes[i]->Points[j].Y < YMin)
							YMin = Courbes[i]->Points[j].Y;
					}
				}
			}

			//Pour régler les axes, évite la division par 0
			if(XMax == XMin)
				XMax += 1;
			if(YMax == YMin)
				YMax += 1;

			//On regle les axes
			Courbes[0]->Points.clear();
			Courbes[0]->Points.push_back(YVec3f(XMin,0,0));
			Courbes[0]->Points.push_back(YVec3f(XMax,0,0));
			Courbes[1]->Points.clear();
			Courbes[1]->Points.push_back(YVec3f(XMin,YMin,0));
			Courbes[1]->Points.push_back(YVec3f(XMin,YMax,0));		
			
			XFactor = (Width-MargeX) / (XMax-XMin);
			YFactor = (Height-MargeY) / (YMax-YMin);

			YVec3f point;
			for(int i=Courbes.size()-1;i>=0;i--)
			{
				Courbes[i]->PointsScaled.clear();
				for(unsigned int j=0;j<Courbes[i]->Points.size();j++)
				{
					point = Courbes[i]->Points[j];

					point.Y -= YMin;
					point.X -= XMin;

					point.X *= XFactor;
					point.Y *= YFactor;
						
					point.Y = -point.Y + (Y + Height-MargeY/2);
					point.X += X+MargeX/2;

					Courbes[i]->PointsScaled.push_back(point);
				}
			}

		}

		sint8 mouseCallback(int x, int y, uint16 click, sint16 wheel, uint16 zorder, bool focusAvailable, uint32 elapsed)
		{
			sint8 mouseForMe = 0;

			if(zorder == ZOrder)
			{
				GUIPanel::mouseCallback(x,y,click,wheel,zorder,focusAvailable,elapsed);

				if(MouseOn)
				{
					float xf = (float) x;
					float yf = (float) y;

					xf -= (X+MargeX/2);
					yf = -yf + (Y + Height-MargeY/2);

					xf /= XFactor;
					yf /= YFactor;

					yf += YMin;
					xf+= XMin;

					char sCoords[1000];
					sprintf_s(sCoords,"X: %0.2f Y: %0.2f",xf,yf);
					_Coords = sCoords;
					mouseForMe = 1;
				}
			}

			return mouseForMe;
		}
		

		//Les courbes 0 et 1 sont les axes 
		void render(uint16 zorder)
		{
			if(Visible && zorder == ZOrder)
			{
				GUIPanel::render(zorder);
				for(unsigned int i=0;i<Courbes.size();i++)
				{
					//Si YMin au dessus de 0, on ne rend pas l'axe des abscisses
					if(YMin>0 && i == 0)
						continue;

					//Si on rend les axes ou si le graphe affiche des courbes
					if(i<=1 || Type == SLP_COURBE)
					{
						glColor3f(Courbes[i]->Color.R,Courbes[i]->Color.V,Courbes[i]->Color.B);
						glBegin(GL_LINE_STRIP);
						for(unsigned int j=0;j<Courbes[i]->PointsScaled.size();j++)
						{				
							glVertex3f(Courbes[i]->PointsScaled[j].X,Courbes[i]->PointsScaled[j].Y,0);						
						}
						glEnd();
					}
					
					//Si c'est pas les axes, et qu'on est en type barres
					if(i > 1 && Type == SLP_BARRES)
					{
						if(Courbes[1]->PointsScaled.size() != 0)
						{
							float yorig = Courbes[1]->PointsScaled[0].Y;
							glColor3f(Courbes[i]->Color.R,Courbes[i]->Color.V,Courbes[i]->Color.B);
							glBegin(GL_QUADS);
							for(unsigned int j=0;j<Courbes[i]->PointsScaled.size();j++)
							{				
								glVertex3f(Courbes[i]->PointsScaled[j].X,Courbes[i]->PointsScaled[j].Y,0);						
								glVertex3f(Courbes[i]->PointsScaled[j].X+5,Courbes[i]->PointsScaled[j].Y,0);
								glVertex3f(Courbes[i]->PointsScaled[j].X+5,yorig,0);
								glVertex3f(Courbes[i]->PointsScaled[j].X,yorig,0);
							}
							glEnd();
						}
					}

					
				}

				//Rendu coords
				uint16 up,down;
				TextEngine->fontHeight(up,down);
				uint16 lenAff = (uint16)_Coords.length() * TextEngine->fontWidth();
				glColor3f(ColorBorder.R,ColorBorder.V,ColorBorder.B);
				glRasterPos2i(X+(Width-lenAff)/2,(int) (Y + Height - (MargeY/2-up)/2));
				TextEngine->glPrint((uint16)_Coords.length(),_Coords.c_str());	
				lenAff = (uint16)Titre.length() * TextEngine->fontWidth();
				glRasterPos2i(X+(Width-lenAff)/2,(int) (Y + (MargeY/2-up)/2));
				TextEngine->glPrint((uint16)Titre.length(),Titre.c_str());	
			}
		}
};