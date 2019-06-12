#pragma once
class VirtualKeyMgr
{
public:
	VirtualKeyMgr(int);
	bool Pressed();
private:
	int  virtual_key;
	int  interval;
	bool blockKey;
};

