#pragma once
#include "chunk.h"

#ifndef YFILE 
#define YFILE 
class MChunk;

class YFile {
public:
	YFile() {

	}

	static void WriteChunk(MChunk * chunk) {
		string path = ToPath(chunk->_XPos, chunk->_YPos, chunk->_ZPos);
		ofstream myfile(path, ios::app);
		for (int x = 0; x < MCubes::CHUNK_SIZE; ++x)
			for (int y = 0; y < MCubes::CHUNK_SIZE; ++y) {
				for (int z = 0; z < MCubes::CHUNK_SIZE; ++z) {
					myfile << chunk->_Cubes->get(x, y, z)->getType();
					if (z == MCubes::CHUNK_SIZE - 1) myfile << " ";
				}
				myfile << "\n";
			}
	}
	static MChunk * ReadChunk(int x, int y, int z) {
		MChunk * chunk = new MChunk(x, y, z);
		if (Exist(x, y, z)) {
			string line;
			ifstream myfile(ToPath(x, y, z));
			while (getline(myfile, line))
			{
				cout << line << '\n';
			}
			myfile.close();
		}
		return chunk;
	}
	static bool Exist(int x, int y, int z) {
		ofstream myfile(ToPath(x, y, z));
		bool exist = myfile.is_open();
		myfile.close();
		return exist;
	}
	static string ToPath(int x, int y, int z) {
		return "chunk" + toString(x) + ":" + toString(y) + ":" + toString(z) + ":" + ".mnc";
	}
};



#endif