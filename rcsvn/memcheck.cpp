#include "memcheck.h"

/* asm for spoofaddy */
TCHAR op_call[50] = { 0 };

/* memcheck globals */
uintptr_t memCheckLoc = 0;
uintptr_t memcheckLoopEnd = 0;
uintptr_t memcheckOriginal = 0;
uintptr_t VmpBase = 0;
uintptr_t VmpClone = 0;
uintptr_t VmpEnd = 0;
uintptr_t textClone = 0;
uintptr_t textEnd = 0;

/* silent checkers */
const uintptr_t crcptr1  = 0x28b40c;
const uintptr_t crcptr2  = 0x2f3d3f;
const uintptr_t crcptr3  = 0x2a5bee;
const uintptr_t crcptr4  = 0x2f1ebd;
const uintptr_t crcptr5  = 0x2f1f60;
const uintptr_t crcptr6  = 0x2f9d3c;
const uintptr_t crcptr7  = 0x2fa85d;
const uintptr_t crcptr8  = 0x2fb6fc;
const uintptr_t crcptr9  = 0x2fc34c;
const uintptr_t crcptr10 = 0x2fce9c;
const uintptr_t crcptr11 = 0x2fe85c;
const uintptr_t crcptr12 = 0x309c30;
const uintptr_t crcptr13 = 0x309d2c;
const uintptr_t crcptr14 = 0x30c90d;
const uintptr_t crcptr15 = 0x311ca7;
const uintptr_t crcptr16 = 0x2f8cec;


/* global spoof ptr */
void(*pSpoofAddy) (void);

/* DISASM */
DISASM crc_check;

__declspec(naked) void SpoofAddy()
{
	__asm
	{
		push ebp
        mov ebp,esp
		// scanned address
		mov eax,[ebp+8] 
        cmp eax, textEnd // is current address outside end of .text
        ja sp_vmp // check if in range of .vmp section
		mov eax, fs:[0x30] 	// .text base from PEB //
		mov eax, [eax + 8]
		cmp [ebp+8], eax // is current address outside start of .text
		jb sp_vmp // check if in range of .vmp section
		// ..in range push .text clone on stack
		push textClone
		// push .text base on stack
        push eax
        mov eax,[ebp+8] // read scanned address
        sub eax,[esp]   // subtract from .text base
        add eax,[esp+4] // add offset to clonebase
        add esp,8
		// return cloned bytes
        movsx eax,byte ptr [eax]
        pop ebp
        ret 4
		sp_vmp:
		mov eax, [ebp + 8] // scanned address
		cmp eax, VmpEnd // is current address outside end of .vmp
		ja sp_exit
		cmp eax, VmpBase // is current address outside start of .vmp
		jb sp_exit
		// ..in range copy .vmp base into eax
        mov eax, VmpBase
		// push .vmp clone on stack
        push VmpClone
		// push .vmp base on stack
        push eax
        mov eax,[ebp+8] // read scanned address
        sub eax,[esp] // subtract from .vmp base
        add eax,[esp+4] // add offset to clonebase
        add esp,8
		// return cloned bytes
        movsx eax,byte ptr [eax]
        pop ebp
        ret 4
        sp_exit:
		// return original bytes if outside .vmp / .text
        mov eax,[ebp+8]
        movsx eax,byte ptr [eax]
        pop ebp
        ret 4
	}
}

__declspec(naked) void mainHook()
{
	__asm
	{
		mov edx, esp
		pop edx
		mov esp, [esp + 8]
		cmp esi, textEnd // is current address outside end of .text
		ja check2
		mov eax, fs: [0x30]	// .text base from PEB //
		mov eax, [eax + 8]
		cmp esi, eax // is current address outside start of .text
		jb check2
		// ..in range, modify stack ptr to avoid detection (stack scanning)
		sub esp, 0x22c
		// push .text clone on stack
		push textClone
		// push .text base on stack
		push eax
		// copy addr being checked into esi
		mov eax, esi
		sub eax, [esp] // subtract addr being checked from .text base to get offset
		add esp, 4
		// add offset to our clone to get real bytes
		add dword ptr[esp], eax
		// jump to main loop, we repeat the above for each addr checked //
		jmp checker
		check2 :
		cmp esi, VmpEnd	// is current address outside end of .vmp
		ja orig_code
		cmp esi, VmpBase // is current address outside start of .vmp
		jb orig_code
		// ..in range, modify stack ptr to avoid detection
		sub esp, 0x22c
		mov eax, VmpBase
		// push .vmp clone on stack
		push VmpClone
		// push .vmp base on stack
		push eax
		// copy addr being checked into esi
		mov eax, esi
		sub eax, [esp] // subtract addr being checked from .vmp base to get offset
		add esp, 4
		// add offset to our clone to get real bytes
		add dword ptr[esp], eax
		// jump to main loop, we repeat the above for each addr checked //
		checker :
		// esp == our clone address //
		mov eax, [esp]	// move clone address into eax
		mov eax, [eax]	// move real bytes from clone
		add eax, esi
		imul eax, eax, 0x1594FE2D
		add eax, [ebp - 0xC]
		rol eax, 0x13
		imul eax, eax, 0xCBB4ABF7
		mov[ebp - 0xC], eax
		lea eax, [esi + 4]
		push eax
		mov eax, [esp + 4]
		mov eax, [eax + 4]
		sub[esp], eax
		pop eax
		add esi, 8
		add dword ptr[esp], 8
		imul eax, eax, 0x344B5409
		add eax, [ebp - 0x14]
		rol eax, 0x11
		imul eax, eax, 0x1594FE2D
		mov dword ptr[ebp - 0x14], eax
		mov eax, [esp]
		mov eax, [eax]
		xor eax, esi
		add esi, 4
		add dword ptr[esp], 4
		imul eax, eax, 0x1594FE2D
		add eax, [ebp - 0x10]
		rol eax, 0xD
		imul eax, eax, 0xCBB4ABF7
		mov dword ptr[ebp - 0x10], eax
		mov eax, [esp]
		mov eax, [eax]
		sub eax, esi
		add esi, 4
		add dword ptr[esp], 4
		imul eax, eax, 0x344B5409
		add eax, ebx
		rol eax, 0xF
		imul ebx, eax, 0x1594FE2D
		cmp esi, edi
		jb checker
		// restore stack ptr //
		add esp, 0x230
		jmp dword ptr ds : [memcheckLoopEnd]
		orig_code :
		jmp dword ptr ds : [memcheckOriginal]
	}
}

void MemCheck::Init()
{
	if (MH_Initialize() != MH_OK)
	{
		return;
	}

	InitializeBeaEngine(); 	// Initialize BeaEngine

	uintptr_t ModBaseAddr = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));

	signature_scanner *newscan = new signature_scanner(GetCurrentProcess(),
														ModBaseAddr, 
														0xf00000);

	memCheckLoc = newscan->search("8b????8b??????????73??8b??????69");

	if (memCheckLoc) 	// null means it's already hooked //
	{
		bypassMainChecker();
		bypassSilentCheckers();
	}

	delete newscan;
}

void MemCheck::bypassMainChecker()
{
	const auto origText = rebase(0x401000);
	size_t textLength;
	textClone = Mv2->cloneSection(origText, &textLength) - 0x1000;

	const auto origVmp = rebase(0x2020000);
	size_t vmpLength;
	VmpClone = Mv2->cloneSection(origVmp, &vmpLength);

	memcheckLoopEnd  = memCheckLoc + calculateJumpOffset(memCheckLoc);
	memcheckOriginal = memCheckLoc + 0x5;

	textEnd = origText + textLength;
	VmpEnd = origVmp + vmpLength;
	VmpBase = origVmp;

	MH_CreateHook(reinterpret_cast<void*>(memCheckLoc), &mainHook, nullptr);
	if (MH_EnableHook(reinterpret_cast<void*>(memCheckLoc)) != MH_OK)
	{
		std::cout << "Failed to enable hook!" << std::endl;
		return;
	}

	Mv2->write_memory(memCheckLoc + 5, "\x90\x90");
}

size_t MemCheck::calculateJumpOffset(uintptr_t addr)
{
	std::string ignoreList[] = {
		"jnc", "jmp" // jumps to ignore
	};

	// size of buffer
	const size_t szBlock = 0x80;

	// Create a new buffer
	void *pBuffer = malloc(szBlock);

	// Nop the buffer
	memset(pBuffer, 0x90, szBlock);

	// Copy instructions to buffer
	memcpy(pBuffer, (void*)*(int*)&addr, szBlock);

	// Initialize Disasm structure
	DISASM DisJump;

	memset(&DisJump, 0, sizeof(DISASM));
	DisJump.Options = Tabulation + MasmSyntax;
	DisJump.Archi = NULL; //IA-32 Architecture//

	// Set Instruction Pointer
	DisJump.EIP = (int)pBuffer;
	DisJump.VirtualAddr = addr;

	// Calculate End Address
	UIntPtr EndCodeSection = (int)pBuffer + szBlock;

	// Disassembly error
	bool DisAsmErr = false;

	while (!DisAsmErr)
	{
		DisJump.SecurityBlock = szBlock;

		size_t len = Disasm(&DisJump);

		if (len == OUT_OF_BLOCK) {
			DisAsmErr = true;
		}
		else if (len == UNKNOWN_OPCODE) {
			DisAsmErr = true;
		}
		else {
			/* if no error -> filter instructions */

			bool ignoreJump = false;  // ignore jumps in blacklist

			if (DisJump.Instruction.BranchType) // check if jump instruction //
			{
				for (const auto& _jumps : ignoreList)
				{
					std::string op = &DisJump.Instruction.Mnemonic[0];

					// remove spaces from mnemonic
					op.erase(remove(op.begin(), op.end(), ' '), op.end());

					if (op == _jumps)
						ignoreJump = true;
				}

				if (!ignoreJump)
				{
					free(pBuffer);
					return ((DisJump.VirtualAddr + len) - addr);
				}
			}

			/* next instruction(s) */

			DisJump.EIP = DisJump.EIP + len;

			DisJump.VirtualAddr = DisJump.VirtualAddr + len;

			if (DisJump.EIP >= EndCodeSection) {
				DisAsmErr = true;
			}
		}
	}

	free(pBuffer);

	return -1;
}

void MemCheck::hookSilentCheckers(uintptr_t adr2hook)
{

	InitializeBeaEngine();

	// size of buffer
	const size_t szBlock = 0x100;

	// Create a new buffer
	void *pBuffer = malloc(szBlock);

	// Nop the buffer
	memset(pBuffer, 0x90, szBlock);

	// Copy instructions to buffer
	memcpy(pBuffer, (void*)*(int*)&adr2hook, szBlock);

	// Set Instruction Pointer
	crc_check.EIP = (int)pBuffer;
	crc_check.VirtualAddr = adr2hook;

	// Calculate End Address
	UIntPtr EndCodeSection = (int)pBuffer + szBlock;

	// Disassembly error
	bool DisAsmErr = false;

	// Allocate mem for the bypass
	void *bypass = VirtualAlloc(nullptr, 2048, MEM_COMMIT, 
								PAGE_EXECUTE_READWRITE
							);

	// reserve mem on stack to avoid detection
	Mv2->write_memory((uintptr_t)bypass,
		"\x81\xEC\x30\x02\x00\x00"); // sub esp, 230 (len: 6)

	//std::cout << bypass << "\n";

	// incrememnt by instruction len
	bypass = (char*)(bypass) + 6;

	// Start of checker
	UIntPtr StartCode = (DWORD)bypass;

	while (!DisAsmErr)
	{
		crc_check.SecurityBlock = szBlock;

		size_t len = Disasm(&crc_check);

		if (len == OUT_OF_BLOCK) {
			DisAsmErr = true;
		}
		else if (len == UNKNOWN_OPCODE) {
			DisAsmErr = true;
		}
		else {

			if (crc_check.Instruction.BranchType)
			{  	/* check for returns and jumps only */
				if (crc_check.Instruction.BranchType == RetType)
				{
					/* copy original code */
					memcpy(bypass, (void*)crc_check.VirtualAddr, len);
					bypass = (char*)(bypass) + len;
					
					/* ret; end of memcheck, stop disasm */
					DisAsmErr = true;
				}
				else {
					/* calculate memcheckLoopStart */
					int memcheckLoopOffset = crc_check.Instruction.AddrValue - adr2hook;

					/* write jne to start of loop */
					Jne((DWORD)bypass,
						(DWORD)StartCode + memcheckLoopOffset,
						bypass);

					/* increment by jne len */
					bypass = (char*)(bypass)+6;
					
					/* restore stack ptr */
					Mv2->write_memory((uintptr_t)bypass,
						"\x81\xC4\x30\x02\x00\x00"); // add esp, 230 (len: 6)

					/* increment by instruction len */
					bypass = (char*)(bypass) + 6;
				}
			}
			/* all other instructions */
			else {
				/* check for memory access */
				std::string op = &crc_check.Instruction.Mnemonic[0];

				/* remove spaces from mnemonic */
				op.erase(remove(op.begin(), op.end(), ' '), op.end());

				if (op == "movsx")
				{
					size_t stublen = 0;

					//std::cout << crc_check.CompleteInstr << "\n";

					/* spoof memcheck code by injecting our stub */
					injectPayloadStub(bypass, stublen);

					bypass = (char*)(bypass) + stublen;
				}
				else {
					/* copy original code */
					memcpy(bypass, (void*)crc_check.VirtualAddr, len);
					bypass = (char*)(bypass) + len;
				}
			}

			/* next instruction(s) */
			crc_check.EIP = crc_check.EIP + len;

			crc_check.VirtualAddr = crc_check.VirtualAddr + len;

			if (crc_check.EIP >= EndCodeSection) {
				DisAsmErr = true;
			}
		}
	}

	/* inject bypass */
	MH_CreateHook(reinterpret_cast<void*>(adr2hook), (char*)StartCode - 6, nullptr);
	if (MH_EnableHook(reinterpret_cast<void*>(adr2hook)) != MH_OK)
	{
		std::cout << "Failed to enable hook!" << std::endl;
	}

	free(pBuffer);
}

void MemCheck::injectPayloadStub(void* bypass, size_t &stublen)
{
	/* destination operand */
	std::string reg1 = &crc_check.Argument1.ArgMnemonic[0]; 

	/* remove spaces from first operand */
	reg1.erase(remove(reg1.begin(), reg1.end(), ' '), reg1.end());

	/* source operand */
	std::string reg2 = &crc_check.Argument2.ArgMnemonic[0];

	/* remove spaces and h suffix from second operand  */
	reg2.erase(std::remove_if(reg2.begin(), reg2.end(),
		[](char chr) { return chr == 'h' || chr == ' '; }), reg2.end()
	);

	/* encapsulate with brackets */
	reg2 = '[' + reg2 + ']';

	/* generate stub based on given args */
	stublen = AsmParse(bypass, reg1, reg2);
}

size_t MemCheck::AsmParse(void* bypass, std::string &r1, std::string &r2)
{
	ks_engine *ks;
	ks_err err;
	size_t count;
	unsigned char *encode;
	size_t size;

	err = ks_open(KS_ARCH_X86, KS_MODE_32, &ks);
	
	if (err != KS_ERR_OK) {
		printf("ERROR: failed on ks_open()\n");
		return -1;
	}

	std::stringstream code;

	if (r1 == "eax")
	{
		code << "lea eax, " + r2 + "\n";
		code << "push eax\n";
		code << op_call;

		if (ks_asm(ks, &code.str()[0], 0, &encode, &size, &count) != KS_ERR_OK) {
			printf("ERROR: ks_asm() failed & count = %lu, error = %u\n",
				count, ks_errno(ks));
			return -1;
		}
		else {
			/* copy stub to bypass */
			memcpy(bypass, (void*)encode, size);
		}
	}
	else {

		code << "push eax\n";
		code << "lea eax, " + r2 + "\n";
		code << "push eax\n";
		code << op_call;
		code << "mov " + r1 + ", eax\n";
		code << "pop eax\n";

		if (ks_asm(ks, &code.str()[0], 0, &encode, &size, &count) != KS_ERR_OK) {
			printf("ERROR: ks_asm() failed & count = %lu, error = %u\n",
				count, ks_errno(ks));
			return -1;
		}
		else {
			/* copy stub to bypass */
			memcpy(bypass, (void*)encode, size);
		}
	}

	// free encode after usage to avoid leaking memory
	ks_free(encode);

	// close Keystone instance when done
	ks_close(ks);

	return size;
}

void MemCheck::Jne(uintptr_t src, uintptr_t dst, void* bypass)
{
	unsigned char buf[] = { 0xF, 0x85, 0x00, 0x00, 0x00, 0x00 };

	unsigned long loc = dst;
	loc = loc - src - sizeof(buf);
	memcpy(buf + 2, &loc, 4);

	memcpy(bypass, &buf, sizeof(buf));
}

void MemCheck::bypassSilentCheckers()
{
	std::vector<uintptr_t> silentCheckers = { crcptr1, crcptr2, crcptr3, crcptr4, crcptr5, crcptr6, crcptr7, crcptr8, crcptr9, crcptr10,
		  crcptr11, crcptr12, crcptr13, crcptr14, crcptr15, crcptr16 };
	  
	pSpoofAddy = &SpoofAddy;

	sprintf(op_call, "call dword ptr ds:[0x%08x]\n", (UIntPtr)&pSpoofAddy);

	for (const auto& _checker : silentCheckers)
	{
		auto checker = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr)) + _checker;

		hookSilentCheckers(checker);
	}
}

void InitializeBeaEngine()
{
	memset(&crc_check, 0, sizeof(DISASM));
	crc_check.Options = Tabulation + MasmSyntax;
	crc_check.Archi = NULL; //IA-32 Architecture//
}