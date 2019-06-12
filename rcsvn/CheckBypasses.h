#include "sigscan.h"
#include "mmgr.h"

// process info
HANDLE processHandle = 0;
DWORD ModBaseAddr = 0;

/*

TEMPLATE

// CheckDescriptionReference
void CheckNameScan()
{
signature_scanner *newscan = new signature_scanner(processHandle, ModBaseAddr, 0xf00000);

uintptr_t CheckName = newscan->search("AOBName");

Mv2->write_memory(CheckName, "\xMemoryReplacement");

delete newscan;

}

*/

// The Parent property of %s is locked, current parent: %s, new parent %s
void Check1Scan()
{
	signature_scanner* newscan = new signature_scanner(processHandle, ModBaseAddr, 0xf00000);

	uintptr_t Check1 = newscan->search("74??837b??0074??8d??????e8");

	Mv2->write_memory(Check1, "\xEB");

	delete newscan;

}

/*
// Invalid parent for Service. %s cannot be parented to %s .
void Check2AScan()
{
signature_scanner *newscan = new signature_scanner(processHandle, ModBaseAddr, 0xf00000);

uintptr_t Check2A = newscan->search("0f85910000008a4722");

Mv2->write_memory(Check2A, "\xE9");

delete newscan;

}

// Invalid parent for Service. %s cannot be parented to %s .
void Check2BScan()
{
signature_scanner *newscan = new signature_scanner(processHandle, ModBaseAddr, 0xf00000);

uintptr_t Check2B = newscan->search("0f84900000006a00e8");

Mv2->write_memory(Check2B, "\xE9");

delete newscan;

}

*/

// Cannot kick a non-local Player from a LocalScript
void Check4Scan()
{
	signature_scanner* newscan = new signature_scanner(processHandle, ModBaseAddr, 0xf00000);

	uintptr_t Check4 = newscan->search("0f848a0000003bb7");

	Mv2->write_memory(Check4, "\x90\xe9");

	delete newscan;

}