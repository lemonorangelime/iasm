#include <stdint.h>

uintptr_t STACK_ADDRESS = 0x70000000;
uintptr_t EXEC_ADDRESS  = 0x7f000000;
uintptr_t JMP_ADDRESS   = 0x01000000;

uintptr_t STACK_SIZE = (4096 * 16);
uintptr_t EXEC_SIZE = 0x00080000;
uintptr_t JMP_SIZE = 0x00080000;

