#pragma once

#include <vector>
#include <string>
#include <cmath>

#define CLASS_OFF		0x10
#define GETPARENT_OFF	0x38
#define PLRHEALTH_OFF	0X1CC
#define GETCHRCTR_OFF	0x5C
#define GCHILDREN_OFF	0x30
#define GETNAME_OFF		0x2C
#define GETLP_OFF		0xCC

// datamodel ptr
#define Game_Ptr_A		0x00d5f174//updated idle for to long
#define Game_Ptr_B		0x01E82884

// workspace ptr
#define gserv_Workspace_Offset	0x1e0

class rinstance
{
public:
	rinstance() {};
	~rinstance() {};
	const char* GetClass(int);
	float GetMagnitude(float, float, float);
	int GetChildByClass(int, const char*);
	int GetChildByName(int, const char*);
	std::vector<int> GetInstanceTable(int);
	std::vector<int> GetChildren(int);
	std::string GetName(int);
	int GetParent(int);
};

struct Vector3 { float x, y, z; };

struct CFrame
{
	float rotationmatrix[9];
	Vector3 position;
};

struct RbxMouse {
	uint32_t mouse_t[2]; 
};
