#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include "memcheck.h"
#include <tchar.h>
#include <string>

#pragma comment(lib, "psapi.lib")

class MemoryManagementV2
{
private:
	HANDLE processHandle;
	DWORD ModBaseAddr;
public:
	MemoryManagementV2();
	DWORD GetBaseAddress();
	uintptr_t cloneSection(uintptr_t, size_t*);
	void Ja(uintptr_t, uintptr_t, void*);
	void Je(uintptr_t, uintptr_t, void*);
	void Jne(uintptr_t, uintptr_t, void*);
	void Jmp(uintptr_t, uintptr_t, void*);
	void write_memory(uintptr_t, unsigned char *, size_t);
	void write_memory(uintptr_t, std::string);
	DWORD Disassemble(uintptr_t, bool _far = true);
	HANDLE GetProcess();
};

extern MemoryManagementV2 *Mv2;
