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
	Input Shift = Input(-112);
	int count = 5;

	Inputs() {
	}
	void keyPressed(int key, bool down, int p1, int p2)
	{
		if (key == Z.keyCode) Z.press = down;
		else if (key == Q.keyCode) Q.press = down;
		else if (key == S.keyCode) S.press = down;
		else if (key == D.keyCode) D.press = down;
		else if (key == Ctrl.keyCode) Ctrl.press = down;
		else if (key == Shift.keyCode) Shift.press = down;
	}


};