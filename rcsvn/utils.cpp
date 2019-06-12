#include "utils.h"

void toClipboard(const std::string &s) {
	OpenClipboard(0);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

void ShowConsole() {
	DWORD dwOldProtect;
	VirtualProtect(&FreeConsole, 1, PAGE_READWRITE, &dwOldProtect);
	*reinterpret_cast<uint8_t*>(FreeConsole) = 0xC3;
	VirtualProtect(&FreeConsole, 1, dwOldProtect, &dwOldProtect);
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
}