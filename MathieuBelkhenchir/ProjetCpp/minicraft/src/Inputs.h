#pragma once
#include "Input.h"
class Inputs
{
public:
	Input Z = Input(122);
	Input Q = Input(113);
	Input S = Input(115);
	Input D = Input(100);
	Input Ctrl = Input(-114);
	Input Space = Input(32);
	Input Shift = Input(-112);
	int count = 5;

	Inputs() {
	}
	void keyPressed(int key, bool down, int p1, int p2)
	{
		//cout << key << endl;
		if (key == Z.keyCode) Z.Press(down);
		else if (key == Q.keyCode) Q.Press(down);
		else if (key == S.keyCode) S.Press(down);
		else if (key == D.keyCode) D.Press(down);
		else if (key == Ctrl.keyCode) Ctrl.Press(down);
		else if (key == Shift.keyCode) Shift.Press(down);
		else if (key == Space.keyCode) Space.Press(down);
	}


};