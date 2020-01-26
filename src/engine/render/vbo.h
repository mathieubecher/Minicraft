#pragma once

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/utils.h" 

class YVbo
{
public:

	//Définit un élément du VBO
	struct Element {
		Element(int nbFloats) 
		{ 
			NbFloats = nbFloats; 
		}
		Element()
		{
			NbFloats = 0;
		}
		int NbFloats = 0; ///< Choisi par l'utilsateur
		int OffsetFloats = 0; ///< Calculé automatiquement
	};

	typedef enum {
		PACK_BY_VERTICE,
		PACK_BY_ELEMENT_TYPE,
	}DATA_STORAGE_METHOD;

private:
	Element * Elements = NULL; ///<La description des différents elements du VBO : une coord de sommet, une normale, une  uv, un float...
	int NbElements; ///< Le nomre d'elements différents qu'on a dans le VBO
	float * ElementsValues = NULL; ///< Des tableaux qui contiennent les valeurs des éléments
	int NbVertices; ///< Le nombre de sommets qu'on a dans le VBO
	int TotalSizeFloats; ///< Taille totale du VBO en floats
	int TotalNbFloatForOneVertice; ///< Taille totale d'un vertice (avec tous ces elements) en floats
	GLuint VBO; ///< L'identifiant du VBO pour opengl
	GLuint VAO; ///< L'identifiant du VAO (description des datas) pour opengl
	DATA_STORAGE_METHOD StorageMethod = PACK_BY_ELEMENT_TYPE; ///< Commen on range les datas dans le VBO

public:		
		
	//On crée un VBO en lui passant les éléments qu'il contient
	YVbo(int nbElements, int nbVertices, DATA_STORAGE_METHOD storageMethod)
	{
		this->Elements = new Element[nbElements];
		NbElements = nbElements;
		NbVertices = nbVertices;
		StorageMethod = storageMethod;
	}

	~YVbo()
	{
		SAFEDELETE_TAB(this->Elements);
		SAFEDELETE_TAB(ElementsValues);
		if (VAO != 0)
			glDeleteVertexArrays(1, &VAO);
		if (VBO != 0)
			glDeleteBuffers(1, &VBO);
	}

	int getVboSizeBytes() {
		return TotalSizeFloats * sizeof(float);
	}

	int getNbVertices() {
		return NbVertices;
	}

	//On set les types d'elements que contient le VBO. On a set le nombre d'elements a la création
	void setElementDescription(int iElement, const Element & element) {
		this->Elements[iElement] = element;

		TotalSizeFloats = 0;
		TotalNbFloatForOneVertice = 0;
		for (int i = 0; i < NbElements; i++) {
			if(StorageMethod == PACK_BY_ELEMENT_TYPE)
				Elements[i].OffsetFloats = TotalSizeFloats;
			else
				Elements[i].OffsetFloats = TotalNbFloatForOneVertice;
				
			TotalNbFloatForOneVertice += Elements[i].NbFloats;
			TotalSizeFloats += NbVertices * Elements[i].NbFloats;
		}
				
	}

	//Création des buffers en RAM pour stoquer toutes les valeurs
	void createVboCpu() {
		SAFEDELETE_TAB(ElementsValues);
		ElementsValues = new float[TotalSizeFloats];
	}

	void deleteVboCpu() {
		SAFEDELETE_TAB(ElementsValues);
		ElementsValues = NULL;
	}

	//Permet de set un element qui fait un float de long
	void setElementValue(int iElement, int iValue, float f1)
	{
		if (StorageMethod == PACK_BY_ELEMENT_TYPE)
			ElementsValues[Elements[iElement].OffsetFloats + iValue] = f1;
		else
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue] = f1;
	}

	//Permet de set un element qui fait 2 float de long
	void setElementValue(int iElement, int iValue, float f1, float f2)
	{
		if (StorageMethod == PACK_BY_ELEMENT_TYPE) {
			ElementsValues[Elements[iElement].OffsetFloats + iValue * 2 + 0] = f1;
			ElementsValues[Elements[iElement].OffsetFloats + iValue * 2 + 1] = f2;
		}
		else {
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue + 0] = f1;
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue + 1] = f2;
		}
			
	}

	//Permet de set un element qui fait 3 float de long
	void setElementValue(int iElement, int iValue, float f1, float f2,float f3)
	{
		if (StorageMethod == PACK_BY_ELEMENT_TYPE) {
			ElementsValues[Elements[iElement].OffsetFloats + iValue * 3 + 0] = f1;
			ElementsValues[Elements[iElement].OffsetFloats + iValue * 3 + 1] = f2;
			ElementsValues[Elements[iElement].OffsetFloats + iValue * 3 + 2] = f3;
		}
		else {
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue + 0] = f1;
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue + 1] = f2;
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue + 2] = f3;
		}
	}

	//Permet de set un element qui fait 4 float de long
	void setElementValue(int iElement, int iValue, float f1, float f2, float f3, float f4)
	{
		if (StorageMethod == PACK_BY_ELEMENT_TYPE) {
			ElementsValues[Elements[iElement].OffsetFloats + iValue * 4 + 0] = f1;
			ElementsValues[Elements[iElement].OffsetFloats + iValue * 4 + 1] = f2;
			ElementsValues[Elements[iElement].OffsetFloats + iValue * 4 + 2] = f3;
			ElementsValues[Elements[iElement].OffsetFloats + iValue * 4 + 3] = f4;
		}
		else {
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue + 0] = f1;
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue + 1] = f2;
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue + 2] = f3;
			ElementsValues[Elements[iElement].OffsetFloats + TotalNbFloatForOneVertice * iValue + 3] = f4;
		}
	}

	//Creation et copie du VBO dans la mémoire du GPU
	void createVboGpu();
	void render();

	
};
