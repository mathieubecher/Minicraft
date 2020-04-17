#pragma once
#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/types_3d.h"

class MPhysics {
public :
	static bool IntersectDroiteQuad(YVec3f Origin, YVec3f PointInDroite, YVec3f aQuad, YVec3f bQuad, YVec3f cQuad, YVec3f dQuad, float & dist) {
		YVec3f point;
		YVec3f planeQuad = (bQuad-aQuad).cross(cQuad-aQuad);

		if (!IntersectDroitePlan(Origin, PointInDroite, planeQuad.X, planeQuad.Y, planeQuad.Z, -(aQuad.X * planeQuad.X + aQuad.Y * planeQuad.Y + aQuad.Z * planeQuad.Z), point))
			return false;
		dist = (point - Origin).getSize() * (((point-Origin).dot(PointInDroite-Origin)>0)?1:-1);
		return PointInQuad(aQuad, bQuad, cQuad, dQuad, point);
	}

	static bool IntersectDroitePlan(const YVec3f& A, const YVec3f& B, const float& a, const float& b, const float& c, const float& d, YVec3f& result) {
		YVec3f dir = B - A;
		float denominateur = (a * dir.X + b * dir.Y + c * dir.Z);
		float nominateur = -(a * A.X + b * A.Y + c * A.Z + d);

		if (denominateur == 0) return false;

		float t = nominateur / denominateur;

		result = A + (B - A) * t;
		
		return !(t < 0 || t > 1);
	}

	static bool PointInQuad(YVec3f a, YVec3f b, YVec3f c, YVec3f d, YVec3f point) {
		YVec3f abp = (b - a).cross(point - a);
		YVec3f bcp = (c - b).cross(point - b);
		YVec3f cdp = (d - c).cross(point - c);
		YVec3f dap = (a - d).cross(point - d);
		return abp.dot(bcp) > 0 && bcp.dot(cdp) > 0 && dap.dot(cdp) > 0;

	}
	static bool testPick(int x, int y, int z, YVec3f lineP1, YVec3f lineP2, float & dist) {
		bool touch = false;

		YVec3f a = YVec3f(x, y, z);
		YVec3f b = YVec3f(x, y + 1, z);
		YVec3f c = YVec3f(x + 1, y + 1, z);
		YVec3f d = YVec3f(x + 1, y, z);
		YVec3f e = YVec3f(x, y + 1, z + 1);
		YVec3f f = YVec3f(x, y, z + 1);
		YVec3f g = YVec3f(x + 1, y, z + 1);
		YVec3f h = YVec3f(x + 1, y + 1, z + 1);


		if ((b - a).cross(c - a).dot(lineP2-lineP1) < 0 && !touch)
			touch |= MPhysics::IntersectDroiteQuad(lineP1, lineP2, a, b, c, d, dist);
		if ((f - e).cross(g - e).dot(lineP2 - lineP1) < 0 && !touch)
			touch |= MPhysics::IntersectDroiteQuad(lineP1, lineP2, e, f, g, h, dist);
		if ((d - a).cross(g - a).dot(lineP2 - lineP1) < 0 && !touch)
			touch |= MPhysics::IntersectDroiteQuad(lineP1, lineP2, a, d, g, f, dist);
		if ((b - c).cross(e - c).dot(lineP2 - lineP1) < 0 && !touch)
			touch |= MPhysics::IntersectDroiteQuad(lineP1, lineP2, c, b, e, h, dist);
		if ((a - b).cross(f - b).dot(lineP2 - lineP1) < 0 && !touch)
			touch |= MPhysics::IntersectDroiteQuad(lineP1, lineP2, b, a, f, e, dist);
		if ((c - d).cross(h - d).dot(lineP2 - lineP1) < 0 && !touch)
			touch |= MPhysics::IntersectDroiteQuad(lineP1, lineP2, d, c, h, g, dist);
		return touch;
	}
};