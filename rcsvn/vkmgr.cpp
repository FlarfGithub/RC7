#include "stdafx.h"
#include "vkmgr.h"

VirtualKeyMgr::VirtualKeyMgr(int VirtualKey)
{
	virtual_key = VirtualKey;
	interval = 0;
	blockKey = false;
}

bool VirtualKeyMgr::Pressed()
{

	DWORD curTick = GetTickCount();

	if (!interval)
		interval = curTick;
	else {
		if (curTick - interval > 600)	// block for half sec //
		{
			interval = curTick;
			blockKey = false;
		}
	}

	if (GetKeyState(virtual_key) & 0x8000)
	{
		if (!blockKey)
		{
			blockKey = true;
			return true;
		}
	}

	return false;
}
