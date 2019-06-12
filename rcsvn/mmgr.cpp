#include "mmgr.h"

MemoryManagementV2::MemoryManagementV2()
{
	this->processHandle = 0;
	this->ModBaseAddr = 0;
}

void MemoryManagementV2::write_memory(uintptr_t addr, unsigned char *patch, size_t dwSize)
{
	DWORD oldProtect;

	VirtualProtect((LPVOID)addr, dwSize,
		PAGE_EXECUTE_READWRITE,
		(PDWORD)&oldProtect);

	for (size_t i = 0; i < dwSize; i++)
		((unsigned char*)addr)[i] = patch[i];

	VirtualProtect((LPVOID)addr, dwSize,
		oldProtect, (PDWORD)&oldProtect);
}

void MemoryManagementV2::write_memory(uintptr_t addr, std::string patch) {
	
	DWORD oldProtect;

	const size_t dwSize = patch.length();

	VirtualProtect((LPVOID)addr, dwSize,
		PAGE_EXECUTE_READWRITE, 
		(PDWORD)&oldProtect);

	for (size_t i = 0; i < dwSize; i++)
		((unsigned char*)addr)[i] = ((unsigned char*)&patch)[i];

	VirtualProtect((LPVOID)addr, dwSize, 
		oldProtect, (PDWORD)&oldProtect);

};

void MemoryManagementV2::Ja(uintptr_t src, uintptr_t dst, void* srcmem)
{
	unsigned char buf[] = { 0xF, 0x87, 0x00, 0x00, 0x00, 0x00 };

	unsigned long loc = dst;
	loc = loc - src - sizeof(buf);
	memcpy(buf + 2, &loc, 4);

	memcpy(srcmem, &buf, sizeof(buf));
}

void MemoryManagementV2::Je(uintptr_t src, uintptr_t dst, void* srcmem)
{
	unsigned char buf[] = { 0xF, 0x84, 0x00, 0x00, 0x00, 0x00 };

	unsigned long loc = dst;
	loc = loc - src - sizeof(buf);
	memcpy(buf + 2, &loc, 4);

	memcpy(srcmem, &buf, sizeof(buf));
}

void MemoryManagementV2::Jne(uintptr_t src, uintptr_t dst, void* srcmem)
{
	unsigned char buf[] = { 0xF, 0x85, 0x00, 0x00, 0x00, 0x00 };

	unsigned long loc = dst;
	loc = loc - src - sizeof(buf);
	memcpy(buf + 2, &loc, 4);

	memcpy(srcmem, &buf, sizeof(buf));
}

void MemoryManagementV2::Jmp(uintptr_t src, uintptr_t dst, void* srcmem)
{
	unsigned char buf[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };

	unsigned long loc = dst;
	loc = loc - src - sizeof(buf);
	memcpy(buf + 1, &loc, 4);

	memcpy(srcmem, &buf, sizeof(buf));
}

DWORD MemoryManagementV2::Disassemble(uintptr_t addr, bool _far)
{
	if (!_far)
	{
		return addr + *(BYTE*)(addr + 1) + 2;
	}

	return addr + *(DWORD*)(addr + 1) + 5;
}

HANDLE MemoryManagementV2::GetProcess()
{
	if (processHandle && (DWORD)processHandle > -1)
		return processHandle;

	this->processHandle = GetCurrentProcess();

	return processHandle;
}

DWORD MemoryManagementV2::GetBaseAddress()
{
	if (ModBaseAddr && ModBaseAddr > - 1)
		return ModBaseAddr;

	this->ModBaseAddr = reinterpret_cast<DWORD>(GetModuleHandle(NULL));

	return ModBaseAddr;
}

uintptr_t MemoryManagementV2::cloneSection(uintptr_t start, size_t* length)
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(reinterpret_cast<void*>(start), &mbi, sizeof(MEMORY_BASIC_INFORMATION));

	auto sectionClone = reinterpret_cast<uintptr_t>(VirtualAlloc(nullptr, mbi.RegionSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	if (!sectionClone)
	{
		return 0;
	}
	std::memcpy(reinterpret_cast<void*>(sectionClone), reinterpret_cast<void*>(start), mbi.RegionSize);

	*length = mbi.RegionSize;
	return sectionClone;
}