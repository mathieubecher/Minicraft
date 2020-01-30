#pragma once
#include "engine/noise/perlin.h"

class YSmartPerlin : public YPerlin {
public:
	static float minz;
	static float maxz;
	virtual float sample(float xBase, float yBase, float zBase)
	{
		if (zBase < minz) return 0;
		if (zBase > maxz + minz) return 1;
		zBase -= minz;
		Freq = 1;
		float actualfreq = 0.015625f;
		float res = YPerlin::sample(xBase*actualfreq,yBase*actualfreq,zBase*actualfreq) * 64;
		actualfreq = 0.03125f;
		res += YPerlin::sample(xBase*actualfreq,yBase*actualfreq,zBase*actualfreq)* 32;
		actualfreq = 0.0625f;
		res += YPerlin::sample(xBase*actualfreq, yBase*actualfreq, zBase*actualfreq) * 16;
		res /= 112;

		float value = (1 - (zBase - minz) / maxz);
		float percent = pow(abs(value - 0.5f) * 2,1);
		return 1- min(1, max(0, res * (1 - percent) + value * percent));
	} 
};
float YSmartPerlin::minz = 20;
float YSmartPerlin::maxz = 60;