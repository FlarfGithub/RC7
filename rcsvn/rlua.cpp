#include "rlua.h"
// Thank you Austin x3

int get_rbxl_opcode(OpCode opcode) {
	return RBXL_OPNAMES[opcode];
}

Instruction ConvertOp(Instruction inst, int k) {
	
	Instruction rtn = ~0;

	OpCode opcode = GET_OPCODE(inst);
	RSET_OPCODE(rtn, get_rbxl_opcode(opcode));

	switch (getOpMode(opcode)) {
	case iABC:
		RSETARG_A(rtn, GETARG_A(inst));
		RSETARG_B(rtn, GETARG_B(inst));
		RSETARG_C(rtn, GETARG_C(inst));
		break;
	case iABx:
		RSETARG_A(rtn, GETARG_A(inst));
		RSETARG_Bx(rtn, GETARG_Bx(inst));
		break;
	case iAsBx:
		RSETARG_A(rtn, GETARG_A(inst));
		RSETARG_sBx(rtn, GETARG_sBx(inst));
		break;
	default:
		return 0;
	}

	if (opcode == OP_MOVE)
		RSETARG_C(rtn, 1);

	return rtn;
}
