#pragma once

#define TOKEN_MAX_SIZE 8

typedef struct printer_token printer_token_t;
typedef struct printer_state printer_state_t;

typedef void (* operator_evaluator_t)(printer_token_t * left, printer_token_t * right);
typedef void (* operator_evaluator_single_t)(printer_token_t * operand);

typedef struct printer_token {
	char buffer[TOKEN_MAX_SIZE];
	int type;
	int precedence;
	int associativity;
	int op_count;
} printer_token_t;

typedef struct printer_state {
	int negative;
} printer_state_t;

enum {
	OPERATOR_ADD,
	OPERATOR_SUB,
	OPERATOR_DIV,
	OPERATOR_MUL,
	OPERATOR_MOD,
	OPERATOR_XOR,
	OPERATOR_LOG_NOT,
	OPERATOR_BIT_NOT,
	OPERATOR_LOG_AND,
	OPERATOR_BIT_AND,
	OPERATOR_LOG_OR,
	OPERATOR_BIT_OR,
	OPERATOR_POW,
	OPERATOR_CMP_GE,
	OPERATOR_CMP_BE,
	OPERATOR_CMP_G,
	OPERATOR_CMP_B,
	OPERATOR_CMP_E,
	OPERATOR_SHR,
	OPERATOR_SHL,
	OPERATOR_OPEN_PARENTHESIS,
	OPERATOR_CLOSE_PARENTHESIS,
};

enum {
	ASSOCIATIVITY_LEFT,
	ASSOCIATIVITY_RIGHT,
};

int print_function(char * operands);
