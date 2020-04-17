#pragma once

#include "engine/utils/types_3d.h"

class YPerlin
{
public:
	YVec3f * Gradients;
	int Width;
	int Size;
	float Freq;

	YPerlin()
	{
		Width = 41;
		Size = Width * Width*Width;
		Gradients = new YVec3f[Size];
		updateVecs();
		Freq = 1;
	}

	void updateVecs() {
		for (int i = 0; i < Size; i++)
		{
			Gradients[i].X = (float)randf();
			Gradients[i].Y = (float)randf();
			Gradients[i].Z = (float)randf();
		}
	}

	float lerp(float a, float b, float alpha) {
		float alphaSmooth = alpha * (3 * alpha - 2 * alpha*alpha);
		return (1 - alphaSmooth)*a + alphaSmooth * b;
	}
public:

	void setFreq(float freq)
	{
		Freq = freq;
	}

	virtual float sample(float xBase, float yBase, float zBase)
	{
		float x = xBase * Freq;
		float y = yBase * Freq;
		float z = zBase * Freq;

		while (x >= Width - 1)
			x -= Width - 1;
		while (y >= Width - 1)
			y -= Width - 1;
		while (z >= Width - 1)
			z -= Width - 1;

		int x1 = (int)floor(x);
		int x2 = (int)floor(x) + 1;
		int y1 = (int)floor(y);
		int y2 = (int)floor(y) + 1;
		int z1 = (int)floor(z);
		int z2 = (int)floor(z) + 1;
		float dx = x - x1;
		float dy = y - y1;
		float dz = z - z1;

		YVec3f pos(x, y, z);
		YVec3f sommets[8];
		//plan X2
		sommets[0] = YVec3f((float)x2, (float)y1, (float)z1);
		sommets[1] = YVec3f((float)x2, (float)y1, (float)z2);
		sommets[2] = YVec3f((float)x2, (float)y2, (float)z2);
		sommets[3] = YVec3f((float)x2, (float)y2, (float)z1);

		//plan X1
		sommets[4] = YVec3f((float)x1, (float)y1, (float)z1);
		sommets[5] = YVec3f((float)x1, (float)y1, (float)z2);
		sommets[6] = YVec3f((float)x1, (float)y2, (float)z2);
		sommets[7] = YVec3f((float)x1, (float)y2, (float)z1);

		float angles[8];
		for (int i = 0; i < 8; i++)
			angles[i] = (pos - sommets[i]).dot(Gradients[(int)(sommets[i].X*Width*Width + sommets[i].Y*Width + sommets[i].Z)]);

		//plan X2
		float ybas = lerp(angles[0], angles[3], dy);
		float yhaut = lerp(angles[1], angles[2], dy);
		float mid2 = lerp(ybas, yhaut, dz);

		//plan X1
		ybas = lerp(angles[4], angles[7], dy);
		yhaut = lerp(angles[5], angles[6], dy);
		float mid1 = lerp(ybas, yhaut, dz);

		float res = lerp(mid1, mid2, dx);

		res = (res + 1) / 2.0f;

		//Milieu
		return min(1, max(0, res));
	}
	float sample(float xBase, float yBase, float zBase,float freq)
	{
		Freq = freq;
		return sample(xBase, yBase, zBase);
	}
};
