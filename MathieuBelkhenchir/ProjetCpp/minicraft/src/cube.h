#pragma once

#include <stdio.h>


class MCube
{
public :
	static MCube Air;

	enum MCubeType	
	{
		CUBE_HERBE = 1,
		CUBE_TERRE,
		CUBE_BOIS,
		CUBE_PIERRE,
		CUBE_EAU,
		CUBE_VERRE,
		CUBE_PLANCHE_01,
		CUBE_PLANCHE_02,
		CUBE_PLANCHE_03,
		CUBE_PLANCHE_04,
		CUBE_PLANCHE_05,
		CUBE_PLANCHE_06,
		CUBE_BRIQUES,
		CUBE_DALLES_01,
		CUBE_DALLES_02,
		CUBE_DALLES_03,
		CUBE_DALLES_04,
		CUBE_SABLE_01,
		CUBE_SABLE_02,
		CUBE_LAINE_01,
		CUBE_LAINE_02,
		CUBE_LAINE_03,
		CUBE_LAINE_04,
		CUBE_LAINE_05,
		CUBE_LAINE_06,
		CUBE_LAINE_07,
		CUBE_LAINE_08,
		CUBE_LAINE_09,
		CUBE_LAINE_10,
		CUBE_LAINE_11,
		CUBE_LAINE_12,
		CUBE_LAINE_13,
		CUBE_LAINE_14,
		CUBE_LAINE_15,
		CUBE_LAINE_16,

		CUBE_CUSTOM_IMAGE,
		CUBE_LIVRE,

		CUBE_TRONC,
		CUBE_BRANCHES,
		CUBE_AIR,

		CUBE_STAIRS,
		CUBE_NB_TYPES
	}; //Limité à 128 types

	static const int CUBE_DRAW_BIT = 0x80;

	private :
		
		
	public :
		uint8 _Code; //premier bit si on doit le draw ou pas, le reste un des 127 types
		YVec3f _NavDir;
		static const int CUBE_SIZE = 1;
		
		MCube()
		{
			setDraw(false);
			setType(CUBE_AIR); 
		}

		void setType(MCubeType type){
			bool draw = getDraw();
			_Code = (uint8)type;
			setDraw(draw);
		}
		void setType(uint8 * type) {
			_Code =*type;
		}


		MCubeType getType(void){
			return (MCubeType)(_Code & ~CUBE_DRAW_BIT);
		}


		bool getDraw(){
			return _Code & CUBE_DRAW_BIT ? true : false;
		}

		void setDraw(bool draw){
			if (draw)
				_Code |= CUBE_DRAW_BIT;
			else
				_Code &= ~CUBE_DRAW_BIT;
		}

		bool isSolid(void)
		{
			MCubeType type = getType();
			return (type != CUBE_AIR && type != CUBE_EAU);
		}

		bool isPickable(void)
		{
			MCubeType type = getType();
			return (type != CUBE_AIR);
		} 

		bool isOpaque(void)
		{
			MCubeType type = getType();
			return  (type != CUBE_AIR && type != CUBE_EAU && type != CUBE_VERRE && type != CUBE_BRANCHES);
		}

		bool isTransparent(void)
		{
			MCubeType type = getType();
			return  (type == CUBE_AIR || type == CUBE_EAU || type == CUBE_VERRE);
		}

		bool isCutoff(void)
		{
			MCubeType type = getType();
			return  (type == CUBE_BRANCHES);
		}

		bool isGround(void)
		{
			MCubeType type = getType();
			return (type == CUBE_HERBE || type == CUBE_TERRE || type == CUBE_EAU || type == CUBE_BOIS || type == CUBE_PIERRE);
		}

		bool isNouricier(void)
		{
			MCubeType type = getType();
			return (type == CUBE_HERBE || type == CUBE_TERRE);
		}

		void saveToFile(FILE * fs)
		{
			fputc(_Code,fs);
		}

		void loadFromFile(FILE * fe)
		{
			_Code = fgetc(fe);
		}

		uint8 getRawCode()
		{
			return _Code;
		}

		void setRawCode(uint8 code){
			_Code = code;
		}

		static string getName(MCubeType type)
		{
			switch (type)
			{
			case CUBE_HERBE: return string("CUBE_HERBE"); break;
			case CUBE_TERRE: return string("CUBE_TERRE"); break;
			case CUBE_BOIS: return string("CUBE_BOIS"); break;
			case CUBE_PIERRE: return string("CUBE_PIERRE"); break;
			case CUBE_EAU: return string("CUBE_EAU"); break;
			case CUBE_VERRE: return string("CUBE_VERRE"); break;
			case CUBE_STAIRS: return string("CUBE_STAIRS"); break;				
			case CUBE_PLANCHE_01: return string("CUBE_PLANCHE_01"); break;
			case CUBE_PLANCHE_02: return string("CUBE_PLANCHE_02"); break;
			case CUBE_PLANCHE_03: return string("CUBE_PLANCHE_03"); break;
			case CUBE_PLANCHE_04: return string("CUBE_PLANCHE_04"); break;
			case CUBE_PLANCHE_05: return string("CUBE_PLANCHE_05"); break;
			case CUBE_PLANCHE_06: return string("CUBE_PLANCHE_06"); break;
			case CUBE_BRIQUES: return string("CUBE_BRIQUES"); break;
			case CUBE_DALLES_01: return string("CUBE_DALLES_01"); break;
			case CUBE_DALLES_02: return string("CUBE_DALLES_02"); break;
			case CUBE_DALLES_03: return string("CUBE_DALLES_03"); break;
			case CUBE_DALLES_04: return string("CUBE_DALLES_04"); break;
			case CUBE_SABLE_01: return string("CUBE_SABLE_01"); break;
			case CUBE_SABLE_02: return string("CUBE_SABLE_02"); break;
			case CUBE_LAINE_01: return string("CUBE_LAINE_01"); break;
			case CUBE_LAINE_02: return string("CUBE_LAINE_02"); break;
			case CUBE_LAINE_03: return string("CUBE_LAINE_03"); break;
			case CUBE_LAINE_04: return string("CUBE_LAINE_04"); break;
			case CUBE_LAINE_05: return string("CUBE_LAINE_05"); break;
			case CUBE_LAINE_06: return string("CUBE_LAINE_06"); break;
			case CUBE_LAINE_07: return string("CUBE_LAINE_07"); break;
			case CUBE_LAINE_08: return string("CUBE_LAINE_08"); break;
			case CUBE_LAINE_09: return string("CUBE_LAINE_09"); break;
			case CUBE_LAINE_10: return string("CUBE_LAINE_10"); break;
			case CUBE_LAINE_11: return string("CUBE_LAINE_11"); break;
			case CUBE_LAINE_12: return string("CUBE_LAINE_12"); break;
			case CUBE_LAINE_13: return string("CUBE_LAINE_13"); break;
			case CUBE_LAINE_14: return string("CUBE_LAINE_14"); break;
			case CUBE_LAINE_15: return string("CUBE_LAINE_15"); break;
			case CUBE_LAINE_16: return string("CUBE_LAINE_16"); break;
			case CUBE_CUSTOM_IMAGE: return string("CUBE_CUSTOM_IMAGE"); break;
			case CUBE_LIVRE: return string("CUBE_LIVRE"); break;
			case CUBE_TRONC: return string("CUBE_TRONC"); break;
			case CUBE_BRANCHES: return string("CUBE_BRANCHES"); break;
			case CUBE_AIR: return string("CUBE_AIR"); break;
			}

			return string("INCONNU");
		}

		static bool isManipulable(MCubeType type)
		{
			switch (type)
			{
			case CUBE_HERBE: 
			case CUBE_TERRE: 
			case CUBE_BOIS:
			case CUBE_PIERRE: 
			case CUBE_EAU: 
			case CUBE_VERRE:
			case CUBE_STAIRS:
			case CUBE_PLANCHE_01: 
			case CUBE_PLANCHE_02: 
			case CUBE_PLANCHE_03: 
			case CUBE_PLANCHE_04:
			case CUBE_PLANCHE_05: 
			case CUBE_PLANCHE_06: 
			case CUBE_BRIQUES: 
			case CUBE_DALLES_01: 
			case CUBE_DALLES_02: 
			case CUBE_DALLES_03: 
			case CUBE_DALLES_04: 
			case CUBE_SABLE_01:
			case CUBE_SABLE_02:
			case CUBE_LAINE_01:
			case CUBE_LAINE_02:
			case CUBE_LAINE_03:
			case CUBE_LAINE_04:
			case CUBE_LAINE_05:
			case CUBE_LAINE_06:
			case CUBE_LAINE_07:
			case CUBE_LAINE_08:
			case CUBE_LAINE_09:
			case CUBE_LAINE_10:
			case CUBE_LAINE_11:
			case CUBE_LAINE_12:
			case CUBE_LAINE_13:
			case CUBE_LAINE_14:
			case CUBE_LAINE_15:
			case CUBE_LAINE_16:
			case CUBE_CUSTOM_IMAGE:
			case CUBE_LIVRE: 
			case CUBE_TRONC: 
					return true;
			}

			return false;
		}
};

MCube MCube::Air = MCube();