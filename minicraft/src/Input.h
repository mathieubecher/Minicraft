#pragma once
class Input
{
public:
	int keyCode;
	bool press;

	Input(int code) {
		press = false;
		keyCode = code;
	}

};