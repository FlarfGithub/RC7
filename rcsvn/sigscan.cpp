
#include "sigscan.h"
#include <locale>         // std::locale, std::toupper
#include <iostream>
#include <sstream>

signature_scanner::signature_scanner(HANDLE hProc, unsigned long RbxMod, unsigned long mem_size)
{

	this->RbxModule = RbxMod;
	this->RbxProc = hProc;
	this->Scanbuf = malloc(mem_size);
	this->ModuleSize = mem_size;
	this->undefined = NULL;

	std::memcpy(Scanbuf, reinterpret_cast<void*>(RbxMod), mem_size);
}

signature_scanner::~signature_scanner() 
{ 
	cleanup(); 
}

bool signature_scanner::err()
{
	if (undefined > 0)
		return true;
	else
		return false;
}

unsigned long signature_scanner::search(std::string aob, size_t offset, bool is_prologue)
{

	if (aob.empty())
		return -1;

	size_t p_len = aob.length();

	// is hex length even?

	if (p_len % 2)
		return -1;

	unsigned short s_length = p_len / 2;

	unsigned char *pattern_buf = new unsigned char[s_length + 1];

	ZeroMemory(pattern_buf, s_length + 1);

	// aob to char array
									   
	hex2bin(&aob[0], pattern_buf);

	// ?? = 2f (used as wildcard by default and ignored by the scanner)
	// if there's actual 2f value in memory, the scanner will ignore the wildcard
	// and treat it as ordinary input

	unsigned short x = 0; // pattern start

	int wildcard, results;

	void *scanned = Scanbuf;

	BYTE mem;

	for (unsigned long i = 0; i < this->ModuleSize; i++)
	{
		wildcard = NULL;
		results  = NULL;

		// scan for pattern starting at first byte...etc //
		for (x = 0; x < s_length; x++)
		{
			memcpy(&mem, (char*)(int)scanned + x, 1);

			if (pattern_buf[x] == 0x2f && mem != 0x2f)	// 2f = ?? (wildcard)
				wildcard++;

			if (pattern_buf[x] == mem) // check for actual hex value //
				results++;

			if (results + wildcard == s_length)
			{
				// pattern match //

				unsigned long matched = (RbxModule + i);

				delete pattern_buf;

				if (is_prologue)
				{
					//-----------------------------------------//
					/* Scan matched for _asm function prolog   */
					//-----------------------------------------//
					/* push ebp								   */
					/* mov ebp, esp 						   */
					//-----------------------------------------//

					// Don't re-scan if we already have the prologue

					if (*(BYTE*)(matched + offset) == 0x55)
						return matched + offset;

					for (size_t idx = 0; idx < 255; idx++)
					{
						uintptr_t _va_ = (matched - idx);

						if (*(BYTE*)_va_ == 0xEC)
							if (*(BYTE*)(_va_ - 1) == 0x8B)
								if (*(BYTE*)(_va_ - 2) == 0x55)
									return _va_ - 2;
					}
				}

				return matched + offset; 
			}
		}
		scanned = (char*)scanned + 1;
	}

	delete pattern_buf;

	++undefined;

	return -1;
}


const char* signature_scanner::stoupper(char *s)
{
	std::string str = s;
	std::stringstream ss;
	std::locale loc;
	
	for (std::string::size_type i = 0; i < str.length(); ++i)
		ss << std::toupper(str[i], loc);

	str = ss.str();

	return str.c_str();
}

int signature_scanner::char2int(char input)
{
	if (input >= '0' && input <= '9')
		return input - '0';
	if (input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if (input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	// not hex, return wildcard
	return '?';
}

void signature_scanner::hex2bin(const char* src, unsigned char* target)
{
	while (*src && src[1])
	{
		*(target++) = char2int(*src) * 16 + char2int(src[1]);
		src += 2;
	}
}

void signature_scanner::cleanup()
{
	free(Scanbuf);
}