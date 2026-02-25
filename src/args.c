#include <iasm/argv.h>
#include <iasm/args.h>
#include <iasm/version.h>
#include <iasm/features.h>
#include <iasm/argvtypes.h>
#include <iasm/types.h>
#include <iasm/asm.h>
#include <iasm/regs.h>
#include <iasm/sdm.h>
#include <iasm/vars.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

void args_feature_callback(void * priv, args_option_t * option, char * arg) {
	if (!arg) { return; } // whatever
	switch (*arg) {
		default:  feature_flags |= decode_feature_flag(arg); return;
		case '+': feature_flags |= decode_feature_flag(arg + 1); return;
		case '-': feature_flags &= ~decode_feature_flag(arg + 1); return;
	}
}

void args_address_callback(void * priv, args_option_t * option, uintptr_t arg, int present) {
	if (!present) { return; }

	if (strcmp(option->long_name, "code-address") == 0 || strcmp(option->long_name, "exec-address") == 0 || strcmp(option->long_name, "text-address") == 0) {
		EXEC_ADDRESS = arg;
		return;
	} else if (strcmp(option->long_name, "stack-address") == 0) {
		STACK_ADDRESS = arg;
		return;
	} else if (strcmp(option->long_name, "trampoline-address") == 0 || strcmp(option->long_name, "jmptable-address") == 0 || strcmp(option->long_name, "jumptable-address") == 0) {
		JMP_ADDRESS = arg;
		return;
	}
}

void args_replay_callback(void * priv, args_option_t * option, char * arg) {
	if (!arg) { return; } // whatever
	replay_file = arg;
}

void args_sdm_callback(iasm_priv_t * priv, args_option_t * option, char * arg) {
	if (!arg) { return; } // whatever
	priv->sdm_query = arg;
}

void args_rgb_style_callback(void * priv, args_option_t * option, char * arg) {
	if (!arg) { return; } // whatever
	rgb_style = decode_rgb_style(arg);
}

void args_type_callback(void * priv, args_option_t * option, char * arg) {
	if (!arg) { return; } // whatever

	int type = decode_type(arg);
	char * option_name = option->long_name;
	if (strcasecmp(option_name, "xmm-type") == 0) {
		xmm_type = type;
	} else if (strcasecmp(option_name, "ymm-type") == 0) {
		ymm_type = type;
	} else if (strcasecmp(option_name, "zmm-type") == 0) {
		zmm_type = type;
	} else if (strcasecmp(option_name, "tmm-type") == 0) {
		tmm_type = type;
	}

}

void args_colour_callback(void * priv, args_option_t * option, int present) {
	if (!present) { return; }
	allow_colour = 0;
}

void args_verbosity_callback(void * priv, args_option_t * option, int present) {
	verbose = present;
}

void args_enable_callback(void * priv, args_option_t * option, int present) {
	if (!present) { return; }

	if (strcmp(option->long_name, "enable-emulation") == 0) {
		feature_flags |= decode_feature_flag("emulation");
	} else if (strcmp(option->long_name, "disable-emulation") == 0) {
		feature_flags &= ~decode_feature_flag("emulation");
	} else if (strcmp(option->long_name, "enable-checkpoints") == 0) {
		feature_flags |= decode_feature_flag("checkpoints");
	} else if (strcmp(option->long_name, "disable-checkpoints") == 0) {
		feature_flags &= ~decode_feature_flag("checkpoints");
	}
}

char version_string[64];
int handle_args(int argc, char * argv[]) {
	snprintf(version_string, 64, "%d.%d.%d", major_version, minor_version, patch_version);

	args_progspec_t spec[] = {
		"IASM", version_string, "Lemon", NULL,
		"unlicense: <https://unlicense.org>",
		"This is public domain software: you are free to change and redistribute it.",
		"There is NO WARRANTY, to the extent permitted by law.",
		"program."
	};
	args_option_t options[] = {
		{'f', "feature",             1, TYPE_STRING,  0, args_feature_callback,   .help="enable / disable feature"},
		{'v', "verbose",             0, TYPE_BOOL,    0, args_verbosity_callback, .help="increase verbosity"},
		{'r', "replay",              1, TYPE_STRING,  0, args_replay_callback,    .help="replay file"},
		{0,   NULL,                  0, TYPE_NULL,    0, NULL,                    .help=""}, // padding
		{0,   "text-address",        1, TYPE_ADDRESS, 0, args_address_callback,   .help=""}, // alias
		{0,   "exec-address",        1, TYPE_ADDRESS, 0, args_address_callback,   .help=""}, // alias
		{'c', "code-address",        1, TYPE_ADDRESS, 0, args_address_callback,   .help="set code memory address (MMU required)"},
		{0,   NULL,                  0, TYPE_NULL,    0, NULL,                    .help=""}, // padding
		{'s', "stack-address",       1, TYPE_ADDRESS, 0, args_address_callback,   .help="set stack memory address (MMU required)"},
		{0,   NULL,                  0, TYPE_NULL,    0, NULL,                    .help=""}, // padding
		{0,   "trampoline-address",  1, TYPE_ADDRESS, 0, args_address_callback,   .help=""}, // alias
		{0,   "jmptable-address",    1, TYPE_ADDRESS, 0, args_address_callback,   .help=""}, // alias
		{'j', "jumptable-address",   1, TYPE_ADDRESS, 0, args_address_callback,   .help="set jump table memory address (MMU required)"},
		{0,   NULL,                  0, TYPE_NULL,    0, NULL,                    .help=""}, // padding
		{'m', "sdm",                 1, TYPE_STRING,  0, args_sdm_callback,       .help="print an Intel SDM entry, then exit"},
		{0,   "isdm",                1, TYPE_STRING,  0, args_sdm_callback,       .help=""}, // alias
		{0,   "man",                 1, TYPE_STRING,  0, args_sdm_callback,       .help=""}, // alias
		{0,   NULL,                  0, TYPE_NULL,    0, NULL,                    .help=""}, // padding
		{'n', "no-colour",           0, TYPE_BOOL,    0, args_colour_callback,    .help="disable ANSI terminal colours"},
		{0,   "no-color",            0, TYPE_BOOL,    0, args_colour_callback,    .help=""}, // alias
		{0,   "rgb-style",           1, TYPE_STRING,  0, args_rgb_style_callback, .help="set RGB printing style"},
		{0,   NULL,                  0, TYPE_NULL,    0, NULL,                    .help=""}, // padding
		{0,   "xmm-type",            1, TYPE_STRING,  0, args_type_callback,      .help="set type of XMM registers"},
		{0,   "ymm-type",            1, TYPE_STRING,  0, args_type_callback,      .help="set type of YMM registers"},
		{0,   "zmm-type",            1, TYPE_STRING,  0, args_type_callback,      .help="set type of ZMM registers"},
		{0,   "tmm-type",            1, TYPE_STRING,  0, args_type_callback,      .help="set type of TMM registers"},
		{0,   NULL,                  0, TYPE_NULL,    0, NULL,                    .help=""}, // padding
		{0,   "enable-emulation",    0, TYPE_BOOL,    0, args_enable_callback,    .help="enable privileged instruction emulation"},
		{0,   "disable-emulation",   0, TYPE_BOOL,    0, args_enable_callback,    .help="disable privileged instruction emulation"},
		{0,   "enable-checkpoints",  0, TYPE_BOOL,    0, args_enable_callback,    .help="enable checkpoints"},
		{0,   "disable-checkpoints", 0, TYPE_BOOL,    0, args_enable_callback,    .help="disable checkpoints"},
		{0,   NULL,                  0, TYPE_NULL,    0, NULL,                    .help=""}, // padding
	};

	int options_count = sizeof(options) / sizeof(options[0]);
	args_setup(ARG_NONE_REQUIRED);
	args_load_spec(spec);

	iasm_priv_t priv = {NULL};
	if (args_parse(argc, argv, options_count, options, &priv)) {
		return -1;
	}

	args_unsetup();
	if (priv.sdm_query) {
		intel_sdm_t * entry = sdm_entry(priv.sdm_query);
		if (!entry) {
			printf("NO ENTRY FOUND.\n");
			return -1;
		}
		sdm_print(entry);
		return -1;
	}

	return 0;
}
