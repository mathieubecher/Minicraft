#pragma once
class Input
{
public:
	int keyCode;
	bool press;
	bool down;

	Input(int code) {
		press = false;
		keyCode = code;
	}
	void Press(bool press) {
		down = press && !this->press;
		this->press = press;
	}
};