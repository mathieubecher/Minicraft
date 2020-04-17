#ifndef __YOCTO_TEXT_ENGINE__
#define __YOCTO_TEXT_ENGINE__

#include <string>
#include "engine/utils/types.h"

#include "gl/gl.h"

#define NB_FONTS 2

class YTextEngine
{
	private:
		//Private GDI
		HDC	_HDC;	
		
		//Fonts
		uint32 _FontList[NB_FONTS];
		TEXTMETRIC _Metrics[NB_FONTS];
		HFONT _Fonts[NB_FONTS];
		int _SelectedFont;

	public:
		YTextEngine (HDC	hdc)
		{
			_HDC = hdc;
			_SelectedFont = 0;
		}

		GLvoid buildFont(int num, int size)							// Build Our Bitmap Font
		{
			HFONT	font;										// Windows Font ID
			HFONT	oldfont;									// Used For Good House Keepinge
		
			if(num >= NB_FONTS)
				return;

			_FontList[num] = glGenLists(255-32);							// Storage For 96 Characters
	
			font = CreateFont(	-size,							// Height Of Font
								0,								// Width Of Font
								0,								// Angle Of Escapement
								0,								// Orientation Angle
								FW_BOLD,						// Font Weight
								FALSE,							// Italic
								FALSE,							// Underline
								FALSE,							// Strikeout
								ANSI_CHARSET,					// Character Set Identifier
								OUT_TT_PRECIS,					// Output Precision
								CLIP_DEFAULT_PRECIS,			// Clipping Precision
								ANTIALIASED_QUALITY,			// Output Quality
								FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
								L"Lucida console");					// Font Name

			_Fonts[num] = font;
			oldfont = (HFONT)SelectObject(_HDC, font);           // Selects The Font We Want
			wglUseFontBitmaps(_HDC, 32, 255-32, _FontList[num]); // Builds 255-32 Characters Starting At Character 32		
			GetTextMetrics(_HDC,&(_Metrics[num]));
			SelectObject(_HDC, oldfont);	//select old font
			DeleteObject(font);									// Delete The Font
		}

		GLvoid KillFont(int num)									// Delete The Font List
		{
			if(num >= NB_FONTS)
				return;
			glDeleteLists(_FontList[num], 255-32);							// Delete All 96 Characters
		}

		void SelectFont(int num)
		{
			if(num >= NB_FONTS)
				return;
			//SelectObject(_HDC, _Fonts[num]);	
			_SelectedFont = num;
		}
	
		GLvoid glPrint(sint16 len, const char *fmt, ...)					// Custom GL "Print" Routine
		{
			char text[1000];								// Holds Our String
			va_list		ap;										// Pointer To List Of Arguments
		
			if (fmt == NULL)									// If There's No Text
				return;											// Do Nothing
		
			va_start(ap, fmt);									// Parses The String For Variables
			vsprintf_s(text, fmt, ap);						// And Converts Symbols To Actual Numbers
			va_end(ap);											// Results Are Stored In Text
		
			glPushAttrib(GL_LIST_BIT); // Pushes The Display List Bits
			glListBase(_FontList[_SelectedFont] - 32);	// Sets The Base Character to 32
			if(len > 0 && len < (sint16) strlen(text))
				glCallLists(len, GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
			else
				glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
			glPopAttrib(); // Pops The Display List Bits
		}

		uint16 stringSize(std::string & chaine)
		{
			return (uint16)(chaine.size() * _Metrics[_SelectedFont].tmAveCharWidth);//8;
		}

		uint16 fontHeight(uint16 & up, uint16 & down)
		{
			up = (uint16) _Metrics[_SelectedFont].tmAscent;//9;
			down = (uint16) _Metrics[_SelectedFont].tmDescent;//2;
			return (uint16) _Metrics[_SelectedFont].tmHeight;//11;
		}

		uint16 fontWidth(void)
		{
			return (uint16)_Metrics[_SelectedFont].tmAveCharWidth;//8;
		}

};

#endif