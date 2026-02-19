#include <iasm/builtins.h>
#include <iasm/helpers.h>
#include <iasm/regs.h>
#include <iasm/asm.h>
#include <iasm/types.h>
#include <iasm/platform.h>
#include <stdio.h>

int print_function(char * regname, char * _type) {
	char buffer[4096];
	int size = 0;
	int type = 0;
	if (resolve_any_register_or_label(regname, buffer, &size, &type)) {
		return 0;
	}

	int type_identifier = *_type ? decode_type(_type) : type;
	print_typed_bytes(buffer, type_identifier, size);
	putchar('\n');
	return 1;
}

