#pragma once

extern "C" {
	#include "lua/lopcodes.h"
}

#include "varint.hpp"	 

/* top level var_arg */
#define ltopVarPtr  26   
/* top level max_stack */
#define ltopStkPtr  27
/* top level opcodes */
#define ltopCodePtr	28   
/* proto upvals */
#define lUpvalsPtr  12
/* source name length */
#define lScnlenPtr  12
/* proto params */
#define lParamsPtr  13  
/* proto var_arg */
#define lProVarPtr  14 
/* proto max_stack */
#define lProStkPtr  15  
/* proto instructions */
#define lproCodePtr 16    

enum R_LuaTypes
{
	R_LUA_TNIL,
	R_LUA_FBOOLEAN,
	R_LUA_TBOOLEAN,
	R_LUA_TNUMBER,
	R_LUA_TSTRING,
	R_LUA_TTHREAD,
	R_LUA_TFUNCTION
};

typedef unsigned char Byte;

const char RBXL_OPNAMES[]{
	0x06, /* MOVE		00 */
	0x04, /* LOADK		01 */
	0x00, /* LOADBOOL	02 */
	0x07, /* LOADNIL	03 */
	0x02, /* GETUPVAL	04 */
	0x08, /* GETGLOBAL	05 */
	0x01, /* GETTABLE	06 */
	0x03, /* SETGLOBAL	07 */
	0x05, /* SETUPVAL	08 */
	0x0F, /* SETTABLE	09 */
	0x0D, /* NEWTABLE	10 */
	0x09, /* SELF		11 */
	0x10, /* ADD		12 */
	0x0B, /* SUB		13 */
	0x11, /* MUL		14 */
	0x0A, /* DIV		15 */
	0x0C, /* MOD		16 */
	0x0E, /* POW		17 */
	0x18, /* UNM		18 */
	0x16, /* NOT		19 */
	0x12, /* LEN		20 */
	0x19, /* CONCAT		21 */
	0x14, /* JMP		22 */
	0x1A, /* EQ			23 */
	0x13, /* LT			24 */
	0x15, /* LE			25 */
	0x17, /* TEST		26 */
	0x21, /* TESTSET	27 */
	0x1F, /* CALL		28 */
	0x1F, /* TAILCALL (0x1B) Not used by VM, instead uses OP_CALL	29 */
	0x22, /* RETURN		30 */ // In the compiled code, this is at the end.
	0x1D, /* FORLOOP	31 */
	0x23, /* FORPREP	32 */
	0x1C, /* TFORLOOP	33 */
	0x1E, /* SETLIST	34 */
	0x20, /* CLOSE		35 */
	0x25, /* CLOSURE	36 */
	0x24, /* VARARG		37 */
};

#define RSET_OPCODE(i,o)	((i) = (((i) & MASK0(6, 26)) | \
		(((Instruction)o << 26) & MASK1(6, 26))))
#define RGET_OPCODE(i)		(i >> 26 & MASK1(6, 0))

#define RSETARG_A(i,o)		((i) = (((i) & MASK0(8, 18)) | \
		(((Instruction)o << 18) & MASK1(8, 18))))
#define RGETARG_A(i)		(i >> 18 & MASK1(8, 0))

#define RSETARG_B(i,o)		((i) = (((i) & MASK0(9, 0)) | \
		(((Instruction)o << 0) & MASK1(9, 0))))
#define RGETARG_B(i)		(i >>  0 & MASK1(9, 0))

#define RSETARG_C(i,o)		((i) = (((i) & MASK0(9, 9)) | \
		(((Instruction)o << 9) & MASK1(9, 9))))
#define RGETARG_C(i)		(i >>  9 & MASK1(9, 0))

#define RSETARG_Bx(i,b)		((i) = (((i) & MASK0(18, 0)) | \
		(((Instruction)b << 0) & MASK1(18, 0))))
#define RGETARG_Bx(i)		(i >>  0 & MASK1(18, 0))

#define RSETARG_sBx(i,b)	RSETARG_Bx((i),cast(unsigned int, (b)+MAXARG_sBx))
#define RGETARG_sBx(i)		(RGETARG_Bx(i)-MAXARG_sBx)

Instruction ConvertOp(Instruction inst, int k);
