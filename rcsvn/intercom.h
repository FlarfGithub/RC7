#pragma once

#include <windows.h>
#include "rinstance.h"
#include "rcsvn.h"
#include "BeaEngine/BeaEngine.h"
#include "keystone/keystone.h"
#include <string>
#include <vector>

DWORD WINAPI MultiHackBase(LPVOID);
uintptr_t findOpCallStart(uintptr_t);
uintptr_t findOpClosureStart(uintptr_t);
uintptr_t findOpSetupvalStart(uintptr_t);
uintptr_t findConditionalStart(uintptr_t);
uintptr_t findConditionalJumpback(uintptr_t);
uintptr_t findOpJmpStart(uintptr_t);
uintptr_t StartNamedPipeServer();
std::vector<uintptr_t> fetchRetCheckOffsets(uintptr_t);
uintptr_t findOpRetStart(uintptr_t);
int *findVMSwitchTable(uintptr_t);
uintptr_t findOpClosureUpvalCheck(uintptr_t);
std::string findSourceOperand(uintptr_t, const char *);
std::string findDestinationOperand(uintptr_t, const char *);
uintptr_t appendLocalVar(void *, uintptr_t);
size_t KsAssembler(const char *, unsigned char *);
void create_inline_func(uintptr_t);
void __fastcall AobScan();

extern DWORD ModBaseAddr;

// definitions
typedef CFrame*(__thiscall *GetMouseHit_Def)(uintptr_t mouse, CFrame *hit);
typedef uintptr_t(__thiscall *GetMouse_Def)(uintptr_t localPlayer, RbxMouse *result);
typedef int(__cdecl *LoadScript_Def)(int r_lua_State, std::string* code, const char* source, int unk);
typedef uintptr_t(__thiscall *MoveTo_Def)(uintptr_t ins, float x, float y, float z);
typedef int*(__cdecl *rluaDumpToProto_Def)(int source, int r_lua_State, const char *lChunkName, int offset);
typedef const char*(__fastcall *r_luaS_newlstr_Def)(int r_lua_State, const char* str, size_t l);
typedef void*(__cdecl *r_luaM_realloc_Def)(int r_lua_State, int block, int osize, int nsize);
typedef int (__stdcall *r_lua_settop_Def)(int r_lua_State, int idx);
typedef int (__cdecl *r_lua_gettop_Def)(int r_lua_State);
typedef int (__cdecl  *r_lua_newthread_Def)(int r_lua_State);
typedef int (__cdecl  *r_lua_resume_Def)(int r_lua_State, int nargs);
typedef int(__stdcall *GetLuaState_Def)();

extern uintptr_t players;
extern uintptr_t character;
extern uintptr_t datamodel;
extern uintptr_t scriptcontext;
extern uintptr_t localPlayer;
extern uintptr_t workspace;
extern uintptr_t mouse;
extern uintptr_t game;

extern GetLuaState_Def GetLuaState;
extern r_lua_settop_Def r_lua_settop;
extern r_lua_gettop_Def r_lua_gettop;
extern rluaDumpToProto_Def rluaDumpToProto;
extern r_lua_newthread_Def r_lua_newthread;
extern r_lua_resume_Def r_lua_resume;
extern r_luaM_realloc_Def r_luaM_realloc;
extern r_luaS_newlstr_Def r_luaS_newlstr;
extern GetMouse_Def GetPlayerMouse;
extern LoadScript_Def LoadScriptT;
extern GetMouseHit_Def GetMouseHit;
extern MoveTo_Def MoveTo;

extern uintptr_t retcheck_1;
extern uintptr_t vm_hook;
extern rinstance ins;