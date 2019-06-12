#pragma once

#include <iostream>

#include <windows.h>
#include <algorithm>
#include <sstream>
#include <vector>

#include "keystone/keystone.h"
#include "MinHook/include/MinHook.h"
#include "BeaEngine/BeaEngine.h"
#include "sigscan.h"

#include "mmgr.h"

#pragma comment(lib, "BeaEngine/lib/BeaEngine.lib")
#pragma comment(lib, "MinHook/lib/libMinHook-x86.lib")
#pragma comment(lib, "keystone/keystone.lib")

static uintptr_t rebase(uintptr_t address)
{
	return address - 0x400000 + reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
}

void InitializeBeaEngine();

namespace MemCheck
{
	void Jne(uintptr_t, uintptr_t, void*);
	void hookSilentCheckers(uintptr_t);
	void injectPayloadStub(void*, size_t&);
	size_t AsmParse(void*, std::string &, std::string &);
	size_t calculateJumpOffset(uintptr_t);
	void bypassSilentCheckers();
	void bypassMainChecker();
	void Init();
}