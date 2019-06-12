#pragma once

#include <windows.h>
#include <string>

class signature_scanner
{
private:
	
	HANDLE RbxProc;

	unsigned long RbxModule, ModuleSize, undefined;

	void *Scanbuf;

public:
	signature_scanner(HANDLE, unsigned long, unsigned long);

	~signature_scanner();

	int char2int(char);
	void hex2bin(const char*, unsigned char*);
	unsigned long search(std::string, size_t offset=0, bool is_prologue = false);
	const char* stoupper(char*);
	void cleanup();
	bool err();
};