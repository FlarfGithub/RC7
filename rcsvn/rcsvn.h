#pragma once

#define WIN32_LEAN_AND_MEAN

#include <mutex>
#include <vector>
#include <iostream>
#include <sstream>
#include <windows.h>  
#include "parallel_hashmap/phmap.h"
#include <random>
#include "intercom.h"
#include "mmgr.h"
#include "rlua.h" 
#include "TSMK2.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/lobject.h"
#include "lua/lstring.h"
#include "lua/lopcodes.h"
#include "lua/lfunc.h"
#include "lua/ldo.h"
}

#pragma comment(lib, "lua5.1-static.lib")

using phmap::flat_hash_map;

namespace RBX
{
	namespace Serializer
	{
		struct HeaderData
		{
			size_t szproto;
			size_t szparams;
			size_t szconst;
			size_t szopcodes;
			size_t szupvals;
		};


		struct l_Proto
		{
			uint32_t sizeop;  // number of instructions
			uint32_t luatype; // constant type
			uint32_t consptr; // constant type ptr
			uint32_t insptr;  // start of instructions
			uint32_t size_sl; // size source lines
			uint32_t sizecs;  // number of constants
			uint32_t upvals;  // number of upvalues
			uint32_t protos;  // number of prototypes
			uint32_t locals;  // number of locals
			uint32_t cslen;   // constant length
			l_Proto();		  // initialization
		};

		class VanillaD
		{
		public:
			VOID *GetChunk();
			DWORD SizeOfChunk();
			VOID WriteHeader(RBX::Serializer::HeaderData *);
			size_t GetLineInfo(const char*);
			VOID ConvertDump(const char*, int*);
			size_t EncryptLines(const char*);
			size_t CopyLocals(const char*);
			size_t CopyUpvals(const char*);
			VOID WriteDump();
			VOID InitDump();
			VanillaD();
		private:
			unsigned long HEADER;	    // ;; LUA_SIGNATURE
			unsigned long Instruction;  // ;; Opcode + Args
			unsigned long Opcode;	    // ;; Opcode
			unsigned long Opmode;	    // ;; Register Type	 
			unsigned char op_enc[8];    // ;; Var_int buffer for no. opcodes
			unsigned char pa_enc[8];	// ;; Var int buffer for no. params
			unsigned char p_enc[8];	    // ;; Var_int buffer for no. protos
			unsigned char c_enc[8];		// ;; Var_int buffer for no. constants
			unsigned char l_enc[8];		// ;; Var_int buffer for no. locals
			unsigned char u_enc[8];		// ;; Var int buffer for no. upvals
			unsigned char k_enc[8];		// ;; Var int buffer for strings
			std::vector<DWORD> op_list; // ;; Instruction list
			std::stringstream  slines;	// ;; Source line positions
			std::stringstream  Kst;		// ;; Constant Table
			size_t szlocout;			// ;; Size locals in bytes
			size_t ldbgptr;				// ;; Lines info ptr
			size_t scnlen;			    // ;; Source name length
			size_t ldumpPtr;		    // ;; Current ptr in RBX dump
			size_t lprotoPtr;		    // ;; Current ptr in proto
			size_t num_op;			    // ;; Number of opcodes
			size_t num_cs;			    // ;; Number of constants
			size_t num_ls;				// ;; Number of locals
			uint64_t sizek;				// ;; Number of strings
			Byte numparams;			    // ;; Parameters
			Byte numupvals;				// ;; Upvalues
			Byte maxstack;			    // ;; Max stack size
			Byte isvararg;			    // ;; Flag _VARARG_
			void *rbx_dump;			    // ;; Bytecode storage
			void *sz_dump;			    // ;; Bytecode size
		};
	}
}

namespace LuaLoad
{
	typedef unsigned char byte;

	static const char kBytecodeMagic[] = "RSB1";

	struct r_TValue {
		int value_0;
		int value_1;
		int tt;
		int unk;
	};

	struct r_LClosure {
		int *next;					// +00  00 
		byte marked;				// +04  04 
		byte tt;					// +05  05 
		byte isC;					// +06  06 
		byte nupvalues;				// +07  07 
		int *gclist;				// +08  08  
		int *unk;					// +0C  12 
		int *env;					// +10  16 
		int *p;						// +14  20
		int upvals[1];				// +20  24 
	};

	struct lDumpMetaData
	{
		const char* source, *eof, *_eof;
	};

	struct lDumpInfo {
		lDumpMetaData *md;
		int unk_1, unk_2;
	};

	struct DeserializedString {
		uintptr_t *result, *unk, *unk2, *unk3;
		size_t szlower, szhigher;
		DeserializedString() : szhigher(15), szlower(0) {}
	};

	void HookUndump();
	void HookVM();
}

typedef uintptr_t(__thiscall *CreateDString_Def)
(LuaLoad::DeserializedString* self, std::string*, int, int);

static void writeVarInt(std::stringstream& ss, uint64_t value) {
	UCHAR buf[8] = { 0 };
	size_t sz_var_out = encodeVarint<uint64_t>(value, buf);
	ss.write(reinterpret_cast<const char*>
		(buf), sz_var_out
	);
}

static void writeDouble(std::stringstream& ss, double value) {
	ss.write(reinterpret_cast<const char*>
		(&value), sizeof(value));
}

static void writeByte(std::stringstream& ss, unsigned char value)
{
	ss.write(reinterpret_cast<const char*>
		(&value), sizeof(value));
}

int Writer(lua_State *L, const void* b, size_t size, void* B);
int str_dump(lua_State *L);
unsigned char r_get_base();
unsigned char r_get_top();

#define r_luaM_malloc(luastate,t)	r_luaM_realloc(luastate, NULL, 0, (t))
#define r_lua_pop(r, n) r_lua_settop(r, -(n)-1)

DWORD WINAPI RCSvnInit(LPVOID);
uint64_t IsDuplicate(std::string luaStr, uint64_t Id);
uint32_t UlongToHexString(uint64_t, char *);
void __stdcall RunScript(void*, size_t);
std::string random_string();

int encrypt_ptr(int, void*);
int decrypt_ptr(int);
int rand_num(int min = 0, int max = 0);

uint64_t IsDuplicate(std::string, uint64_t);
uint64_t OpDuplicate(std::string);