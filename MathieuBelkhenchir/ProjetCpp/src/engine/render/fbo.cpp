#pragma once

#include "engine/render/fbo.h"
#include "engine/render/renderer.h"
#include "engine/render/tex_manager.h"

void YFbo::setColorAsShaderInput(int numCol, int location, const char * texSamplerName)
{
	GLint prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

	GLuint texLoc = glGetUniformLocation(prog, texSamplerName);
	YRenderer::checkGlError("glGetUniformLocation(prog, texSamplerName);");
	glUniform1i(texLoc, location - GL_TEXTURE0);
	YRenderer::checkGlError("glUniform1i(texLoc, location- GL_TEXTURE0);");

	glActiveTexture(location);
	glBindTexture(GL_TEXTURE_2D, ColorTex[numCol]);

	glActiveTexture(GL_TEXTURE0); 
}

void YFbo::setDepthAsShaderInput(int location, const char * texSamplerName)
{
	GLint prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

	GLuint texLoc = glGetUniformLocation(prog, texSamplerName);
	YRenderer::checkGlError("glGetUniformLocation(prog, texSamplerName);");

	glUniform1i(texLoc, location - GL_TEXTURE0);
	YRenderer::checkGlError("glUniform1i(texLoc, location- GL_TEXTURE0);");

	glActiveTexture(location);
	glBindTexture(GL_TEXTURE_2D, DepthTex);
	
	glActiveTexture(GL_TEXTURE0);
}



void YFbo::setAsOutFBO(bool set, bool clear)
{
	if (set)
	{
		//On passe en FBO pour pouvoir faire nos effets
		glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
		YRenderer::checkGlError("glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_FBO);");

		//Attach 2D texture to this FBO
		for (int i = 0; i < this->NbColorTex; i++) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, ColorTex[i], 0);
			YRenderer::checkGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _ColorTex[i], 0);");
		}

		//Attach depth texture to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTex, 0);
		YRenderer::checkGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_depth_tex, 0);");
	}
	else
	{
		//On passe en mode rendu normal
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	//Erase
	if (clear) {
		glClearColor(YRenderer::getInstance()->BackGroundColor.R,
			YRenderer::getInstance()->BackGroundColor.V,
			YRenderer::getInstance()->BackGroundColor.B,
			YRenderer::getInstance()->BackGroundColor.A);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}


void YFbo::createColorTexs(int width, int height)
{
	if (ColorTex[0] != 0)
		glDeleteTextures(NbColorTex, ColorTex);

	glGenTextures(NbColorTex, ColorTex);

	for (int i = 0; i < NbColorTex; i++)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ColorTex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f); //no aniso filtering
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}

	YRenderer::checkGlError("createColorTexs");
}

void YFbo::createDepthTex(int width, int height)
{
	if (DepthTex != 0)
		glDeleteTextures(1, &DepthTex);

	glGenTextures(1, &DepthTex);
		
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DepthTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//NULL means reserve texture memory, but texels are undefined
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	YRenderer::checkGlError("createDepthTex");

}

//// !! Creer les tex avnt de creer les FBO
void YFbo::createFBO()
{
	if (FBO != 0)
		glDeleteFramebuffers(1, &FBO);

	glGenFramebuffers(1, &FBO);

	//On bind le FBO pour tester si tout est ok
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//Attach 2D texture to this FBO
	for (int i = 0; i < NbColorTex; i++)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, ColorTex[i], 0);

	//Attach depth texture to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTex, 0);

	//Does the GPU support current FBO configuration?
	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		YLog::log(YLog::ENGINE_INFO, "GPU ok for FBO with depth and color");
		break;
	default:
		YLog::log(YLog::ENGINE_ERROR, "GPU does not support FBO");
		return;
	}

	//On debind
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void YFbo::readFb(uint8 * buff)
{
	glReadPixels((GLint)0, (GLint)0,
		(GLint)Width, (GLint)Height,
		GL_RGB, GL_UNSIGNED_BYTE, buff);
}

void YFbo::readFbTex(int numCol, uint8 * buff, uint32 bufSize)
{
	int width, height;

	if (!buff) {
		YLog::log(YLog::ENGINE_ERROR, "Fbo read fail, buf is null");
		return;
	}

	uint32 nbPixels = Width * Height;
	if (nbPixels * 3 != bufSize) {
		YLog::log(YLog::ENGINE_ERROR, "Fbo read fail buffer has bad size");
		return;
	}

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, ColorTex[numCol]);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	if (width != Width || height != Height) {
		YLog::log(YLog::ENGINE_ERROR, "Fbo tex has not buff size, cannot read it");
		return;
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1); //par defaut word aligned et padding deborde ?	

																			 /*int val;
																			 glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_INTERNAL_FORMAT, &val);
																			 glGetIntegerv(GL_PACK_ALIGNMENT, &val);*/

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buff);
	YRenderer::checkGlError("glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buff);");

	glBindTexture(GL_TEXTURE_2D, 0);

}

void YFbo::saveFbToFile(const char * name, int width, int height)
{
	uint32 nbPixels = width * height;
	uint8 * pixelsRgb = (uint8*)malloc(nbPixels * 3);

	if (!pixelsRgb) {
		YLog::log(YLog::ENGINE_ERROR, "FB save to file alloc failed");
		return;
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels((GLint)0, (GLint)0,
		(GLint)width, (GLint)height,
		GL_RGB, GL_UNSIGNED_BYTE, pixelsRgb);


	YTexManager::writeImage(name, width, height, pixelsRgb, "FB save", true);

	free(pixelsRgb);
}

void YFbo::saveTexToFile(const char * name, int numCol)
{
	uint32 nbPixels = Width * Height;
	uint8 * pixelsRgb = (uint8*)malloc(nbPixels * 3);

	if (!pixelsRgb) {
		YLog::log(YLog::ENGINE_ERROR, "Fbo save to file alloc failed");
		return;
	}

	readFbTex(numCol, pixelsRgb, nbPixels * 3);

	YTexManager::writeImage(name, Width, Height, pixelsRgb, "fbo save", true);

	free(pixelsRgb);
}


