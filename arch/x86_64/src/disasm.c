#include <Zydis.h>
#include <stdint.h>
#include <string.h>

int disasm_one(void * p, char * buffer) {
	ZyanU64 runtime_address = (uintptr_t) p;
	ZyanUSize offset = 0;
	ZydisDisassembledInstruction instruction;

	ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, (uintptr_t) p, p, 0xff, &instruction);
	strcpy(buffer, instruction.text);
	return instruction.info.length;
}
