#pragma once

#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include "tskptrs.h"
#include "intercom.h"
#include <windows.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

enum Call_T
{
	_thisCall,
	_stdCall
};

namespace RBX
{

	struct Task
	{
		uintptr_t *function;
		Call_T type;
		std::vector<void*> args;
		bool active;
	};

	namespace TaskScheduler
	{
		struct Job {
			void *vtable;
			int unk_0, unk_1, unk_2;
			std::string name;
			int padding[4];
			double elapsed;
		};
	}

	class TSMK2Resolver
	{
	public:
		TSMK2Resolver();
		VOID ThrottlePacketJob();
		DWORD GetPacketRecvJob(std::vector <std::shared_ptr<const RBX::TaskScheduler::Job>>&);
		DWORD GetMemoryCheckerJob(std::vector <std::shared_ptr<const RBX::TaskScheduler::Job>>&);
		std::vector <std::shared_ptr<const RBX::TaskScheduler::Job>>& GetJobQueue();
		void __fastcall addTask(uintptr_t*, std::vector<void*>, Call_T);
		VOID SwapVTable(std::shared_ptr <const RBX::TaskScheduler::Job>, size_t);
		VOID SwapVTable(std::shared_ptr <const RBX::TaskScheduler::Job>, size_t, uintptr_t*);
		VOID HookMemCheckVFT(std::shared_ptr <const RBX::TaskScheduler::Job>, size_t);
		DWORD GetDummyJob(size_t vecptr = QueVec2Ptr);
		int GetVirtualFunction(void *, int) const;
		void EnableMemCheck();
		void DisableMemCheck();
		void BypassChecks();
		void ResumeChecks();
		bool RetChkDisabled();
		bool PktRcvDisabled();
		VOID ReinitializeHook();
		VOID HookPacketRecvJob();
		VOID HookScriptJob();
		VOID HookMemoryJob();
		VOID ResetVars();
	private:
		bool retchk;
		bool recvpkts;
		bool TsException;
		std::shared_ptr<const RBX::TaskScheduler::Job> packetRecvJob, genericJob;
		size_t queue_loc;
		int pktrecv;
	};
}

extern RBX::TSMK2Resolver *t2r;

DWORD WINAPI TSMK2Throttler(LPVOID);
DWORD WINAPI MessageBoxThread(LPVOID);
DWORD WINAPI JobHook(LPVOID);
void __fastcall TeleportImpl();
void querytasks();