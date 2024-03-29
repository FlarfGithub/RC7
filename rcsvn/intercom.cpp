#include "CheckBypasses.h"
#include "stdafx.h"
#include <fstream>
#include <iostream>
#include "intercom.h"
#include "rinstance.h"
#include "sigscan.h"
#include "rcsvn.h"
#include "vkmgr.h"
#include "TSMK2.h"
#include "utils.h"
#include "mmgr.h"

// globals
uintptr_t players = 0;
uintptr_t character = 0;
uintptr_t scriptcontext = 0;
uintptr_t datamodel = 0;
uintptr_t localPlayer = 0;
uintptr_t workspace = 0;
uintptr_t mouse = 0;
uintptr_t game = 0;

// found by signature scanner
r_luaS_newlstr_Def r_luaS_newlstr = 0;
r_lua_settop_Def r_lua_settop = 0;
r_lua_gettop_Def r_lua_gettop = 0;
r_lua_newthread_Def r_lua_newthread = 0;
r_lua_resume_Def r_lua_resume = 0;
GetMouse_Def GetPlayerMouse = 0;
GetLuaState_Def GetLuaState = 0;
rluaDumpToProto_Def rluaDumpToProto = 0;
r_luaM_realloc_Def r_luaM_realloc = 0;
GetMouseHit_Def GetMouseHit = 0;
LoadScript_Def LoadScriptT = 0;
MoveTo_Def MoveTo = 0;

// memlib
MemoryManagementV2 *Mv2;

// checkers 
uintptr_t retcheck_1 = 0;
uintptr_t vm_hook = 0;

rinstance ins;

VOID InitOffsets()
{
	try {

		datamodel = ModBaseAddr + Game_Ptr_A;
		datamodel = *reinterpret_cast<uintptr_t*>(datamodel);

		if (!datamodel)	/* check datamodel validity */
		{
			datamodel = ModBaseAddr + Game_Ptr_B;
			datamodel = *reinterpret_cast<uintptr_t*>(datamodel);
		}
		std::cout << "Datamodel: " << std::hex << datamodel << "\n";
		workspace = datamodel + gserv_Workspace_Offset;
		workspace = *reinterpret_cast<uintptr_t*>(workspace);

		if (!workspace) /* check workspace validity */
		{
			datamodel = ModBaseAddr + Game_Ptr_B;
			datamodel = *reinterpret_cast<uintptr_t*>(datamodel);
			workspace = datamodel + gserv_Workspace_Offset;
			workspace = *reinterpret_cast<uintptr_t*>(workspace);
		}
		std::cout << "Workspace: " << std::hex << workspace << "\n";
		game = ins.GetParent(workspace); 

		players = (uintptr_t)ins.GetChildByClass(game, "Players");

		scriptcontext = (uintptr_t)ins.GetChildByClass(game, "ScriptContext");

		std::cout << "Players: " << std::hex << players << "\n";
		std::cout << "ScriptContext: " << std::hex << scriptcontext << "\n";

		localPlayer = *(uintptr_t*)(players + GETLP_OFF);

		std::cout << "LocalPlayer: " << std::hex << localPlayer << "\n";
	}
	catch (...)
	{
		std::cout << "An error occurred during initiating offsets: " << GetLastError() << "\n";
	}

}

void ExecuteScript(const char *buffer)
{
	lua_State * L = luaL_newstate();
	luaL_openlibs(L);

	std::string luaScript = buffer;

	int err = luaL_loadstring(L, luaScript.c_str());
	RBX::Serializer::VanillaD *Rs = new RBX::Serializer::VanillaD;

	if (!err)
	{
		str_dump(L);
		const char *dump = lua_tostring(L, -1);
		Rs->InitDump();
		Rs->ConvertDump(dump, NULL);

		size_t szDump = Rs->SizeOfChunk();
		void* r_lDump = Rs->GetChunk();

		t2r->addTask((uintptr_t*)RunScript, { (void*)szDump, r_lDump }, _stdCall);

		//std::cout << r_lDump << "\n";

		lua_pop(L, 2);
	}
	else {
		const char* err_msg = lua_tostring(L, -1);
		std::cout << err_msg << "\n";
		lua_pop(L, 1);
	}

	lua_gc(L, LUA_GCCOLLECT, 0);
	lua_close(L);
	delete Rs;
}

std::string ReadFiles(std::string dir) {
	std::ifstream file(dir.c_str());
	std::string str;
	std::string Filecont;
	while (std::getline(file, str))
	{
		Filecont += str + "\n";
	}
	return Filecont;
}

UINT StartNamedPipeServer()
{
	HANDLE hPipe;
	const size_t szBuf = 10000;
	char *buffer = new char[szBuf];
	DWORD dwRead;

	memset(buffer, 0, szBuf);

	hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\RC7_SCRIPT"),
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
		1,
		1024 * 16,
		1024 * 16,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);

	while (hPipe != INVALID_HANDLE_VALUE)
	{
		if (ConnectNamedPipe(hPipe, NULL) != FALSE)   // wait for someone to connect to the pipe
		{
			while (ReadFile(hPipe, buffer, szBuf - 1, &dwRead, NULL) != FALSE)
			{
				buffer[dwRead] = '\0';
				std::string Script = ReadFiles("C:/rcseven/Script.lua");
				ExecuteScript(Script.c_str());
			}
		}

		DisconnectNamedPipe(hPipe);
	}

	return 0;
}

DWORD WINAPI MultiHackBase(LPVOID)
{

	Mv2 = new MemoryManagementV2;

	t2r = new RBX::TSMK2Resolver();

	ModBaseAddr = Mv2->GetBaseAddress();

	processHandle = Mv2->GetProcess();

	InitOffsets();

	AobScan();

	CreateThread(NULL, NULL, JobHook,
		NULL, NULL, NULL);

	CreateThread(NULL, NULL, TSMK2Throttler,
		NULL, NULL, NULL);

	t2r->DisableMemCheck();
	MemCheck::Init();
	t2r->EnableMemCheck();

	Check1Scan();
	Check4Scan();

	RCSvnInit(0);

	StartNamedPipeServer();

	return 0;
}

std::vector<uintptr_t> fetchRetCheckOffsets(uintptr_t func2check)
{
	
	// size of buffer
	const size_t szBlock = 0x300;

	// Create a new buffer
	void *pBuffer = malloc(szBlock);

	// Nop the buffer
	memset(pBuffer, 0x90, szBlock);

	// Copy instructions to buffer
	memcpy(pBuffer, (void*)*(int*)&func2check, szBlock);

	// Initialize Disasm structure
	DISASM DisRet;

	memset(&DisRet, 0, sizeof(DISASM));
	DisRet.Options = Tabulation + MasmSyntax;
	DisRet.Archi = NULL; //IA-32 Architecture//

	// Set Instruction Pointer
	DisRet.EIP = (int)pBuffer;
	DisRet.VirtualAddr = func2check;

	// Return ptrs
	std::vector<uintptr_t> Retchecks_v;

	// Calculate End Address
	UIntPtr EndCodeSection = (int)pBuffer + szBlock;

	// Disassembly error
	bool DisAsmErr = false;

	while (!DisAsmErr)
	{
		DisRet.SecurityBlock = szBlock;

		size_t len = Disasm(&DisRet);

		if (len == OUT_OF_BLOCK) {
			DisAsmErr = true;
		}
		else if (len == UNKNOWN_OPCODE) {
			DisAsmErr = true;
		}
		else {
			std::string op = &DisRet.Instruction.Mnemonic[0];

			if (op.substr(0,3) == "cmp")
			{

				UCHAR results[15] = { 0 };

				memcpy(&results, (void*)DisRet.VirtualAddr, sizeof(results));

				/* magic signature: 3b??????????72??a1 */

				if (results[0] == 0x3b)
					if (results[6] == 0x72)
						if (results[8] == 0xa1)
						{
							uintptr_t retCheckOffset = (uintptr_t)DisRet.VirtualAddr + 6;
							retCheckOffset -= func2check;
							Retchecks_v.push_back(retCheckOffset);
						}
			}

			if (DisRet.Instruction.BranchType == RetType)
				DisAsmErr = true;

			/* next instruction(s) */
			DisRet.EIP = DisRet.EIP + len;

			DisRet.VirtualAddr = DisRet.VirtualAddr + len;

			if (DisRet.EIP >= EndCodeSection) {
				DisAsmErr = true;
			}
		}
	}

	free(pBuffer);

	if (Retchecks_v.empty())
		return { 0 };
	else
		return Retchecks_v;
}

void create_inline_func(uintptr_t _inline_f)
{

	void *GetLStateClone =  VirtualAlloc(nullptr, 1024, MEM_COMMIT,
										PAGE_EXECUTE_READWRITE
							);

	GetLuaState = (GetLuaState_Def)GetLStateClone;

	// get the offset for lua state
	unsigned char ls_offset = *(BYTE*)(_inline_f + 12);

	// create a buffer to convert code to str
	TCHAR ls_buf[200] = { 0 };
	
	UCHAR *asm_buf = new UCHAR[200];

	// pad buffer with nops
	memset(asm_buf, 0x90, 200);

	/* create stub to retrieve lua state */
	std::string code = "pushad\n";
	code += "mov " + findSourceOperand(_inline_f + 16, NULL) + ", 0x%08x\n";
	code += "mov " + findDestinationOperand(_inline_f + 16, NULL) + ", 0x%08x\n";
	/* pad with nops, we will add more code later */
	code += "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n";
	code += "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"; 
	code += "ret\n";

	sprintf(ls_buf, code.c_str(), ls_offset, scriptcontext);

	size_t opsize = KsAssembler(ls_buf, asm_buf);

	// write memory parsed by keystone

	Mv2->write_memory((uintptr_t)GetLuaState,
						asm_buf, opsize);

	uintptr_t *globalState = new uintptr_t[4]; 
	*globalState = 0;

	// append original code to prologue

	const size_t szOrig = 7;

	Mv2->write_memory((uintptr_t)GetLuaState + 11,
		(UCHAR*)(_inline_f + 16), szOrig);

	// append epilogue to close out function

	code.clear();

	code =  "pop dword ptr ds:[0x%08x]\n";
	code += "popad\n";
	code += "mov eax, ds:[0x%08x]\n";

	ZeroMemory(ls_buf, sizeof(ls_buf));
	sprintf(ls_buf, code.c_str(), globalState, globalState);

	opsize = KsAssembler(ls_buf, asm_buf);

	// write memory parsed by keystone

	Mv2->write_memory((uintptr_t)GetLuaState + 11 + szOrig,
						asm_buf, opsize);

	delete asm_buf;
}

size_t KsAssembler(const char *code, unsigned char *out)
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

	if (ks_asm(ks, code, 0, &encode, &size, &count) != KS_ERR_OK) {
		printf("ERROR: ks_asm() failed & count = %lu, error = %u\n",
			count, ks_errno(ks));
		return -1;
	}
	else {
		memcpy(out, encode, size);
	}

	// free encode after usage to avoid leaking memory
	ks_free(encode);

	// close Keystone instance when done
	ks_close(ks);

	return size;
}

std::string findSourceOperand(uintptr_t v_addr, const char *opCmp)
{
	// size of buffer
	const size_t szBlock = 0x80;

	// Create a new buffer
	void *pBuffer = malloc(szBlock);

	// Nop the buffer
	memset(pBuffer, 0x90, szBlock);

	// Copy instructions to buffer
	memcpy(pBuffer, (void*)*(int*)&v_addr, szBlock);

	// Initialize Disasm structure
	DISASM DisReg;

	memset(&DisReg, 0, sizeof(DISASM));
	DisReg.Options = Tabulation + MasmSyntax;
	DisReg.Archi = NULL; //IA-32 Architecture//

	// Set Instruction Pointer
	DisReg.EIP = (int)pBuffer;
	DisReg.VirtualAddr = v_addr;

	// Calculate End Address
	UIntPtr EndCodeSection = (int)pBuffer + szBlock;

	// Disassembly error
	bool DisAsmErr = false;

	while (!DisAsmErr)
	{
		DisReg.SecurityBlock = szBlock;

		size_t len = Disasm(&DisReg);

		if (len == OUT_OF_BLOCK) {
			DisAsmErr = true;
		}
		else if (len == UNKNOWN_OPCODE) {
			DisAsmErr = true;
		}
		else {
			/* if no error -> filter instructions */

			std::string op = &DisReg.Instruction.Mnemonic[0];

			/* remove spaces from mnemonic */
			op.erase(remove(op.begin(), op.end(), ' '), op.end());

			if (opCmp == NULL)
			{
				std::string r1 = &DisReg.Argument1.ArgMnemonic[0];

				/* remove spaces from first operand */
				r1.erase(remove(r1.begin(), r1.end(), ' '), r1.end());

				free(pBuffer);

				return r1;
			}

			if (op == opCmp)
			{
				/* destination operand */
				std::string r1 = &DisReg.Argument1.ArgMnemonic[0];

				/* remove spaces from first operand */
				r1.erase(remove(r1.begin(), r1.end(), ' '), r1.end());

				free(pBuffer);
				
				return r1;
			}

			/* next instruction(s) */

			DisReg.EIP = DisReg.EIP + len;

			DisReg.VirtualAddr = DisReg.VirtualAddr + len;

			if (DisReg.EIP >= EndCodeSection) {
				DisAsmErr = true;
			}
		}
	}

	free(pBuffer);

	return NULL;
}

std::string findDestinationOperand(uintptr_t v_addr, const char *opCmp)
{
	// size of buffer
	const size_t szBlock = 0x80;

	// Create a new buffer
	void *pBuffer = malloc(szBlock);

	// Nop the buffer
	memset(pBuffer, 0x90, szBlock);

	// Copy instructions to buffer
	memcpy(pBuffer, (void*)*(int*)&v_addr, szBlock);

	// Initialize Disasm structure
	DISASM DisReg;

	memset(&DisReg, 0, sizeof(DISASM));
	DisReg.Options = Tabulation + MasmSyntax;
	DisReg.Archi = NULL; //IA-32 Architecture//

	// Set Instruction Pointer
	DisReg.EIP = (int)pBuffer;
	DisReg.VirtualAddr = v_addr;

	// Calculate End Address
	UIntPtr EndCodeSection = (int)pBuffer + szBlock;

	// Disassembly error
	bool DisAsmErr = false;

	while (!DisAsmErr)
	{
		DisReg.SecurityBlock = szBlock;

		size_t len = Disasm(&DisReg);

		if (len == OUT_OF_BLOCK) {
			DisAsmErr = true;
		}
		else if (len == UNKNOWN_OPCODE) {
			DisAsmErr = true;
		}
		else {
			/* if no error -> filter instructions */

			std::string op = &DisReg.Instruction.Mnemonic[0];

			/* remove spaces from mnemonic */
			op.erase(remove(op.begin(), op.end(), ' '), op.end());

			if (opCmp == NULL)
			{
				std::string r1 = &DisReg.Argument2.ArgMnemonic[0];

				/* remove spaces from first operand */
				r1.erase(remove(r1.begin(), r1.end(), ' '), r1.end());

				free(pBuffer);

				return r1;
			}

			if (op == opCmp)
			{
				/* destination operand */
				std::string r1 = &DisReg.Argument2.ArgMnemonic[0];

				/* remove spaces from first operand */
				r1.erase(remove(r1.begin(), r1.end(), ' '), r1.end());

				free(pBuffer);

				return r1;
			}

			/* next instruction(s) */

			DisReg.EIP = DisReg.EIP + len;

			DisReg.VirtualAddr = DisReg.VirtualAddr + len;

			if (DisReg.EIP >= EndCodeSection) {
				DisAsmErr = true;
			}
		}
	}

	free(pBuffer);

	return NULL;
}

uintptr_t appendLocalVar(void *op_closure_hook, uintptr_t op_closure_loc)
{
	signature_scanner *newscan = new signature_scanner(processHandle, op_closure_loc, 0xf00);

	int _vars_loc = newscan->search("8b??fc");

	const size_t sz_ops_cpy = 3;

	if (_vars_loc > 1)
	{
		Mv2->write_memory((uintptr_t)op_closure_hook, (unsigned char*)_vars_loc, sz_ops_cpy);
		
		return sz_ops_cpy;  // size of instructions copied
	}

	delete newscan;

	return 0;
}

uintptr_t findConditionalJumpback(uintptr_t op_lt_loc)
{
	signature_scanner *newscan = new signature_scanner(processHandle, op_lt_loc, 0xf00);

	int lt_jumpback = newscan->search("33??e9????????8b??8b");

	if (lt_jumpback > 1)
	{
		return Mv2->Disassemble(lt_jumpback + 2); // skip to jump instruction	(e9)
	}

	delete newscan;

	return 0;
}

uintptr_t findOpJmpStart(uintptr_t op_jmp_loc)
{
	signature_scanner *newscan = new signature_scanner(processHandle, op_jmp_loc, 0xf00);

	int op_jmp_start = newscan->search("8b??25000000??3d");

	if (op_jmp_start > 1)
	{
		return op_jmp_start;
	}

	delete newscan;

	return 0;
}

uintptr_t findConditionalStart(uintptr_t op_lt_loc)
{
	signature_scanner *newscan = new signature_scanner(processHandle, op_lt_loc, 0xf00);

	int conditonal = newscan->search("83??200f87");

	if (conditonal > 1)
	{
		return conditonal;
	}

	delete newscan;

	return 0;
}

uintptr_t findOpSetupvalStart(uintptr_t op_setupval_loc)
{
	signature_scanner *newscan = new signature_scanner(processHandle, op_setupval_loc, 0xf00);

	int op_setupval_start = newscan->search("8b??c1??12");

	if (op_setupval_start > 1)
	{
		return op_setupval_start;
	}

	delete newscan;

	return 0;
}

uintptr_t findOpCallStart(uintptr_t op_call_loc)
{
	signature_scanner *newscan = new signature_scanner(processHandle, op_call_loc, 0xf00);

	int op_call_start = newscan->search("81??0000007c8b");

	if (op_call_start > 1)
	{
		return op_call_start;
	}

	delete newscan;

	return 0;
}

uintptr_t findOpClosureUpvalCheck(uintptr_t cl_upvals_loc)
{
	signature_scanner *newscan = new signature_scanner(processHandle, cl_upvals_loc, 0xf00);

	int _check_upvals = newscan->search("837d??1075");

	if (_check_upvals > 1)
	{
		return _check_upvals;
	}

	delete newscan;

	return 0;
}

uintptr_t findOpClosureStart(uintptr_t op_closure_loc)
{
	signature_scanner *newscan = new signature_scanner(processHandle, op_closure_loc, 0xf00);

	int op_closure_start = newscan->search("81??000000948b");

	if (op_closure_start > 1)
	{
		return op_closure_start;
	}

	delete newscan;

	return 0;
}

uintptr_t findOpRetStart(uintptr_t OP_RETURN_HOOK)
{
	// find beginning of op_return by disassembling a jump within its switch

	int op_ret_start = 0;

	for (size_t i = 0; i < 200; i++)
	{
		op_ret_start = (OP_RETURN_HOOK + i);

		if (*(BYTE*)op_ret_start == 0x0f)
		{
			if (*(BYTE*)(op_ret_start + 1) == 0x87)
			{
				break;
			}
		}
	}

	return op_ret_start;
}

int *findVMSwitchTable(uintptr_t vm_loc)
{
	signature_scanner *newscan = new signature_scanner(processHandle, vm_loc, 0xf00);

	int *switch_loc = (int*)newscan->search("ff24??????????8b????81??ff01");

	if ((int)switch_loc > 1)
	{
		switch_loc = (int*)*(int*)((char*)switch_loc + 3);
		return switch_loc;
	}
	
	delete newscan;

	return 0;
}

void __fastcall AobScan()
{
	signature_scanner *newscan = new signature_scanner(processHandle, ModBaseAddr, 0xf00000);

	MoveTo = (MoveTo_Def)newscan->search("f3??????088b??106a0183", -0x5d);
	
	GetMouseHit = (GetMouseHit_Def)newscan->search("83ec14??8b??????8b??ff90??0000008b", -0x18);

	GetPlayerMouse = (GetMouse_Def)newscan->search("c745f00000000083bb????000000??8d", -0x1e);

	uintptr_t _check = Mv2->Disassemble((uintptr_t)GetPlayerMouse + 0x47);

	auto stackchk = fetchRetCheckOffsets(_check);

	retcheck_1 = _check + stackchk[1];

	// std::cout << std::hex << GetPlayerMouse << "\n" << std::hex << MoveTo << "\n" << std::hex << GetMouseHit << "\n";

	// std::cout << std::hex << (*(int*)retcheck_1 & 0xff) << " should be 0x72 or 0xeb\n";
							  
	delete newscan;

	return;
}