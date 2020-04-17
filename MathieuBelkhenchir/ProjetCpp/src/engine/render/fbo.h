#pragma once

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/utils.h" 
#include "engine/render/vbo.h" 
#include "engine/log/log.h"

class YFbo
{
public:
	GLuint * ColorTex;
	int NbColorTex;
	GLuint DepthTex;
	GLuint FBO;
	int Width;
	int Height;
	float SizeDiv;
	bool HasDepth;
	bool PixelArt;

	YFbo(bool depth = true, int nbOutTex = 1, float sizeDiv = 1.0f, bool pixelArt = false)
	{
		ColorTex = new GLuint[nbOutTex];
		memset(ColorTex, 0x00, sizeof(GLuint) *nbOutTex);
		NbColorTex = nbOutTex;
		DepthTex = 0;
		FBO = 0;
		SizeDiv = sizeDiv;
		HasDepth = depth;
		PixelArt = pixelArt;
	}

	~YFbo()
	{
		SAFEDELETE_TAB(ColorTex);
	}

	void init(int width, int height)
	{
		Width = (int)floor(width / SizeDiv);
		Height = (int)floor(height / SizeDiv);
		createColorTexs(Width, Height);
		if (HasDepth)
			createDepthTex(Width, Height);
		createFBO();
	}

	void resize(int width, int height) {
		init(width, height);
	}

	void readFb(uint8 * buff);
	void readFbTex(int numCol, uint8 * buff, uint32 bufSize);
	static void saveFbToFile(const char * name, int width, int height);
	void saveTexToFile(const char * name, int numCol);

	void setColorAsShaderInput(int numCol = 0, int location = GL_TEXTURE0, const char * texSamplerName = "colorTex1");
	void setDepthAsShaderInput(int location, const char * texSamplerName = "depthTex");
	void setAsOutFBO(bool set, bool clear = true);

private:
	void createColorTexs(int width, int height);
	void createDepthTex(int width, int height);
	void createFBO();

};

