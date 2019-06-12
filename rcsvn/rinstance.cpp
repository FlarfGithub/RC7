#include "rinstance.h"

std::string rinstance::GetName(int instance) {
	return (const char*)(*(int*)(instance + GETNAME_OFF));
};

int rinstance::GetParent(int instance) {
	return *(int*)(instance + GETPARENT_OFF);
}

std::vector<int> rinstance::GetInstanceTable(int ptr) {
	std::vector<int> rtn;

	if (!ptr || !*(int*)ptr)
		return rtn;

	int stop_loc = *(int*)(ptr + 4);
	int it = *(int*)ptr;

	while (it != stop_loc) {
		rtn.push_back(*(int*)it);
		it += 8;
	}

	return rtn;
}

std::vector<int> rinstance::GetChildren(int instance) {
	return GetInstanceTable(*(int*)(instance + GCHILDREN_OFF));
}


const char* __fastcall GetInsClassName(int instance)
{
	__asm
	{
		mov eax, [ecx];
		call dword ptr[eax + CLASS_OFF];
	}
}

const char* rinstance::GetClass(int instance) {
	return GetInsClassName(instance);
}

int rinstance::GetChildByClass(int instance, const char* className) {
	std::vector<int> children = GetChildren(instance);

	for (size_t i = 0; i < children.size(); ++i)
	{
		if (strcmp(GetClass(children[i]), className) == 0)
		{
			return children[i];
		}
	}

	return 0;
};

int rinstance::GetChildByName(int instance, const char* name) {
	std::vector<int> children = GetChildren(instance);

	for (size_t i = 0; i < children.size(); ++i) {
		if (GetName(children[i]) == name) {
			return children[i];
		}
	}

	return 0;
};

float rinstance::GetMagnitude(float x, float y, float z)
{
	float magnitude = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

	return abs(magnitude);
}