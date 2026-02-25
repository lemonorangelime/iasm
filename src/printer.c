#include <iasm/printer.h>
#include <iasm/builtins.h>
#include <iasm/helpers.h>
#include <iasm/regs.h>
#include <iasm/asm.h>
#include <iasm/types.h>
#include <iasm/checkpoints.h>
#include <iasm/platform.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_TOKENS 128

#define ctoken(t) (&(char[]) {t, 0, 0, 0, 0, 0, 0, 0})

#define operator_function_innard(operation) \
	if (left->type == FLOAT64 || right->type == FLOAT64) { \
		/* this is really slow... and (more possibly importantly) really really stupid */ \
		double l = left->type == FLOAT64 ? (*(double *) left->buffer) : ((double) *(uint64_t *) left->buffer); \
		double r = right->type == FLOAT64 ? (*(double *) right->buffer) : ((double) *(uint64_t *) right->buffer); \
		double * lb = &l; \
		double * rb = &r; \
		operation; \
		*(double *) left->buffer = *lb; \
		left->type = FLOAT64; \
		return; \
	} \
	uint64_t * lb = (uint64_t *) left->buffer; \
	uint64_t * rb = (uint64_t *) right->buffer; \
	operation;

#define operator_function_innard_single(operation) \
	if (operand->type == FLOAT64) { \
		double bv = operand->type == FLOAT64 ? (*(double *) operand->buffer) : ((double) *(uint64_t *) operand->buffer); \
		double * b = &bv; \
		operation; \
		*(double *) operand->buffer = *b; \
		return; \
	} \
	uint64_t * b = (uint64_t *) operand->buffer; \
	*b = !*b; \
	operation;

static int printer_isdigit(int c) {
	return (c >= '0') && (c <= '9');
}

static int printer_isbin(int c) {
	return (c >= '0') && (c <= '1');
}

static int printer_isoct(int c) {
	return (c >= '0') && (c <= '7');
}

static int printer_ishex(int c) {
	c = tolower(c);
	return ((c >= '0') && (c <= '9')) || (c >= 'a' && c <= 'f');
}

static int printer_isop(int c) {
	switch (c) {
		case '^': case '&': case '*': case '/':
		case '+': case '-': case '|': case '>':
		case '<': case '=': case '~': case '%':
		case '!': case '(': case ')': case ',':
			return 1;
	}
	return 0;
}

static int token_length(char * token) {
	int len = 0;
	while (*token && !isspace(*token) && !printer_isop(*token)) {
		len++;
		token++;
	}
	return len;
}

static char * skip_whitespace(char * s) {
	while (*s && isspace(*s)) { s++; }
	return s;
}

static int utf_match(char * t, char * c) {
	return memcmp(t, c, strlen(c)) == 0;
}

// TODO: this
static uint64_t safe_call_func(uint64_t addr, printer_token_t * sp, int out_count, int operand_count) {
	//uint64_t operand_array[32] = {0};
	//checkpoint_load();
	return 0; // :shrug:
}



void push_token(printer_token_t ** stack, void * value, int size, int associativity, int precedence, int type, int * count) {
	printer_token_t * op = *stack;
	(*count)++;
	memcpy(op->buffer, value, size);
	op->type = type;
	op->precedence = precedence;
	op->associativity = associativity;
	op->op_count = 0;
	(*stack)++;
}

int token_is_constant(char * token) {
	if (utf_match(token, "π") || utf_match(token, "Π")) {
		return 1;
	}
	return 0;
}

int read_constant(char * token, int length, char * buffer, int * type, int sign) {
	if (utf_match(token, "π") || utf_match(token, "Π")) {
		*(double *) buffer = 3.14159265358979323846 * (double) sign;
		*type = FLOAT64;
		return 1;
	}
	return 0;
}

int token_is_hex(char * token, int length) {
	int i = 0;
	while (*token && printer_ishex(*token)) { i++; token++; }
	return i == length;
}

int token_is_oct(char * token, int length) {
	int i = 0;
	while (*token && printer_isoct(*token)) { i++; token++; }
	return i == length;
}

int token_is_bin(char * token, int length) {
	int i = 0;
	while (*token && printer_isbin(*token)) { i++; token++; }
	return i == length;
}

int token_is_float(char * token, int length) {
	int i = 0;
	int dots = 0;
	while (*token && printer_isdigit(*token) || *token == '.') {
		dots += *token == '.';
		i++;
		token++;
	}
	return (i == length) && dots == 1;
}

int token_is_type(char * token) {
	int length = token_length(token);
	if (length == 0 || length > 32) {
		return 0; // can't possibly be a type identifier
	}

	char buffer[33] = {0};
	memcpy(buffer, token, length);
	buffer[length] = 0;
	if (decode_type(buffer) == IASM_TYPE_NULL) {
		return 0;
	}
	return 1;
}

int token_is_number(char * token) {
	int is_constant = token_is_constant(token);
	if (is_constant) {
		return is_constant;
	}
	int length = token_length(token);
	if (length == 0) {
		return 0; // ??
	}
	if (length > 2) {
		if (token[0] == '0' && tolower(token[0]) == 'x') {
			return token_is_hex(token + 2, length - 2);
		}
		if (token[0] == '0' && tolower(token[0]) == 'o') {
			return token_is_oct(token + 2, length - 2);
		}
		if (token[0] == '0' && tolower(token[0]) == 'b') {
			return token_is_bin(token + 2, length - 2);
		}
	}
	int i = 0;
	char * base = token;
	while (*token && printer_isdigit(*token)) {
		i++;
		token++;
	}
	if (*token && *token == '.') {
		return token_is_float(base, length);
	}
	return i == length;
}

int check_next_char(char * token, char c) {
	char n = *(token + 1);
	if (c == 0) {
		return token_is_number(token + 1);
	}
	return n == c;
}

int read_number(char * token, printer_state_t * state, printer_token_t ** output, int * output_count) {
	int length = token_length(token);
	char * token_end = token + length;
	char buffer[8] = {0};
	int type = INT64;
	int neg = state->negative;
	uint64_t sign = state->negative ? -1 : 1;

	if (neg) {
		state->negative = 0;
	}

	if (read_constant(token, length, buffer, &type, sign)) {
		push_token(output, buffer, 8, ASSOCIATIVITY_LEFT, 1, type, output_count);
		return length;
	}

	if (token[0] == '0' && tolower(token[0]) == 'x' && token_is_hex(token + 2, length - 2)) {
		*(uint64_t *) buffer = strtoll(token, &token_end, 16) * sign;
	} else if (token[0] == '0' && tolower(token[0]) == 'o' && token_is_oct(token + 2, length - 2)) {
		*(uint64_t *) buffer = strtoll(token, &token_end, 8) * sign;
	} else if (token[0] == '0' && tolower(token[0]) == 'b' && token_is_bin(token + 2, length - 2)) {
		*(uint64_t *) buffer = strtoll(token, &token_end, 2) * sign;
	} else if (token_is_float(token, length)) {
		*(double *) buffer = strtod(token, &token_end) * (neg ? -1.0 : 1.0);
		type = FLOAT64;
	} else {
		*(uint64_t *) buffer = strtoll(token, &token_end, 10) * sign;
	}

	push_token(output, buffer, 8, ASSOCIATIVITY_LEFT, 1, type, output_count);
	return length;
}

int read_special(char * token, printer_state_t * state, printer_token_t ** output, int * output_count) {
	int length = token_length(token);
	char name[257] = {0};
	strncpy(name, token, length);
	name[length] = 0;

	char buffer[4096] = {0};
	int size = 0;
	int type = 0;
	int reassignable = 0;
	if (resolve_any_register_or_label(name, buffer, &size, &type, &reassignable)) {
		return 0;
	}

	if ((size > 8 && size != 10) || (type != INT64 && type != INT32 && type != INT16 && type != INT8 && type != FLOAT80 && type != FLOAT64)) {
		return 0;
	}

	switch (type) {
		case INT64:
		case INT32:
		case INT16:
		case INT8:
			type = INT64;
			break;
		case FLOAT80:
			*(double *) buffer = fpu_float_to_double((fpu_float_t *) buffer);
		case FLOAT64:
			type = FLOAT64;
			break;
	}


	push_token(output, buffer, 8, ASSOCIATIVITY_LEFT, 1, type, output_count);
	return length;
}

int read_type(char * token, printer_state_t * state, printer_token_t ** op_stack, int * operator_count) {
	int length = token_length(token);
	char buffer[33] = {0};
	memcpy(buffer, token, length);
	buffer[length] = 0;

	int type = decode_type(buffer);
	push_token(op_stack, &type, 4, ASSOCIATIVITY_RIGHT, 128, IASM_TYPE_TYPE, operator_count);
	return length;
}

void handle_operator(int associativity, int precedence, printer_token_t ** out_stack, printer_token_t ** op_stack, int * out_count, int * op_count) {
	printer_token_t * out = *out_stack;
	printer_token_t * op = *op_stack;
	while (*op_count) {
		printer_token_t * o = --op;
		if (o->precedence > precedence || (o->precedence == precedence && associativity == ASSOCIATIVITY_LEFT)) {
			*(*out_stack)++ = *o;
			(*out_count)++;
			(*op_stack)--;
			(*op_count)--;
			continue;
		}
		break;
	}
}

int token_is_function(printer_token_t * o) {
	return o->type == IASM_TYPE_FUNCTION;
}

int op_stack_pop_parenth(printer_token_t ** out_stack, printer_token_t ** op_stack, int * out_count, int * op_count) {
	printer_token_t * out = *out_stack;
	printer_token_t * op = *op_stack;
	while (*op_count) {
		printer_token_t * o = --op;
		if ((*(uint64_t *) o->buffer == OPERATOR_OPEN_PARENTHESIS) && o->type == IASM_TYPE_OPERATOR) {
			(*out_count)++;
			(*op_stack)--;
			(*op_count)--;
			if (*op_count && token_is_function(--o)) {
				*(*out_stack)++ = *o;
				(*out_count)++;
				(*op_stack)--;
				(*op_count)--;
			}
			return 0;
		}
		*(*out_stack)++ = *o;
		(*out_count)++;
		(*op_stack)--;
		(*op_count)--;
	}
	return -1;
}

// return: consumed chars
int read_token(char * token, printer_state_t * state, printer_token_t ** outputp, printer_token_t ** operatorp, int * output_count, int * operator_count) {
	if (token_is_number(token)) {
		return read_number(token, state, outputp, output_count);
	}
	if (token_is_type(token)) {
		return read_type(token, state, operatorp, operator_count);
	}
	int r = read_special(token, state, outputp, output_count);
	if (r > 0) {
		return r;
	}
	printer_token_t * output = *outputp;
	printer_token_t * op = *operatorp;
	switch (*token) {
		case ',': return 1;
		case '^':
			handle_operator(ASSOCIATIVITY_LEFT, 9, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_XOR), 1, ASSOCIATIVITY_LEFT, 9, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '&':
			if (check_next_char(token, '&')) {
				handle_operator(ASSOCIATIVITY_LEFT, 11, outputp, operatorp, output_count, operator_count);
				push_token(operatorp, ctoken(OPERATOR_LOG_AND), 11, ASSOCIATIVITY_LEFT, 2, IASM_TYPE_OPERATOR, operator_count);
				return 2;
			}
			handle_operator(ASSOCIATIVITY_LEFT, 8, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_BIT_AND), 1, ASSOCIATIVITY_LEFT, 8, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '*':
			if (check_next_char(token, '*')) {
				handle_operator(ASSOCIATIVITY_RIGHT, 4, outputp, operatorp, output_count, operator_count);
				push_token(operatorp, ctoken(OPERATOR_POW), 1, ASSOCIATIVITY_RIGHT, 4, IASM_TYPE_OPERATOR, operator_count);
				return 2;
			}
			handle_operator(ASSOCIATIVITY_LEFT, 3, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_MUL), 1, ASSOCIATIVITY_LEFT, 3, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '/':
			handle_operator(ASSOCIATIVITY_LEFT, 3, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_DIV), 1, ASSOCIATIVITY_LEFT, 3, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '+':
			handle_operator(ASSOCIATIVITY_LEFT, 2, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_ADD), 1, ASSOCIATIVITY_LEFT, 2, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '-':
			if (check_next_char(token, 0)) {
				state->negative = 1;
				return 1;
			}
			handle_operator(ASSOCIATIVITY_LEFT, 2, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_SUB), 1, ASSOCIATIVITY_LEFT, 2, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '|':
			if (check_next_char(token, '|')) {
				handle_operator(ASSOCIATIVITY_LEFT, 12, outputp, operatorp, output_count, operator_count);
				push_token(operatorp, ctoken(OPERATOR_LOG_OR), 1, ASSOCIATIVITY_LEFT, 12, IASM_TYPE_OPERATOR, operator_count);
				return 2;
			}
			handle_operator(ASSOCIATIVITY_LEFT, 10, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_BIT_OR), 1, ASSOCIATIVITY_LEFT, 10, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '>':
			handle_operator(ASSOCIATIVITY_LEFT, 2, outputp, operatorp, output_count, operator_count);
			if (check_next_char(token, '=')) {
				push_token(operatorp, ctoken(OPERATOR_CMP_GE), 1, ASSOCIATIVITY_LEFT, 2, IASM_TYPE_OPERATOR, operator_count);
				return 2;
			}
			if (check_next_char(token, '>')) {
				push_token(operatorp, ctoken(OPERATOR_SHR), 1, ASSOCIATIVITY_LEFT, 2, IASM_TYPE_OPERATOR, operator_count);
				return 2;
			}
			push_token(operatorp, ctoken(OPERATOR_CMP_G), 1, ASSOCIATIVITY_LEFT, 2, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '<':
			if (check_next_char(token, '=')) {
				handle_operator(ASSOCIATIVITY_LEFT, 6, outputp, operatorp, output_count, operator_count);
				push_token(operatorp, ctoken(OPERATOR_CMP_BE), 1, ASSOCIATIVITY_LEFT, 6, IASM_TYPE_OPERATOR, operator_count);
				return 2;
			}
			if (check_next_char(token, '<')) {
				handle_operator(ASSOCIATIVITY_LEFT, 5, outputp, operatorp, output_count, operator_count);
				push_token(operatorp, ctoken(OPERATOR_SHL), 1, ASSOCIATIVITY_LEFT, 5, IASM_TYPE_OPERATOR, operator_count);
				return 2;
			}
			handle_operator(ASSOCIATIVITY_LEFT, 6, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_CMP_B), 1, ASSOCIATIVITY_LEFT, 6, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '=':
			// this shit dont make sense
			handle_operator(ASSOCIATIVITY_LEFT, 6, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_CMP_E), 1, ASSOCIATIVITY_LEFT, 6, IASM_TYPE_OPERATOR, operator_count);
			if (check_next_char(token, '=')) {
				return 2;
			}
			return 1;
		case '~':
			handle_operator(ASSOCIATIVITY_RIGHT, 13, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_BIT_NOT), 1, ASSOCIATIVITY_RIGHT, 13, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '%':
			handle_operator(ASSOCIATIVITY_LEFT, 3, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_MOD), 1, ASSOCIATIVITY_LEFT, 3, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '!':
			handle_operator(ASSOCIATIVITY_RIGHT, 13, outputp, operatorp, output_count, operator_count);
			push_token(operatorp, ctoken(OPERATOR_LOG_NOT), 1, ASSOCIATIVITY_RIGHT, 13, IASM_TYPE_OPERATOR, operator_count);
			return 1;
		case '(': push_token(operatorp, ctoken(OPERATOR_OPEN_PARENTHESIS), 1, ASSOCIATIVITY_RIGHT, 1, IASM_TYPE_OPERATOR, operator_count); return 1;
		case ')':
			op_stack_pop_parenth(outputp, operatorp, output_count, operator_count);
			return 1;
	}
	return -1; // UNKNOWN TOKEN
}

int operators_present(printer_token_t * stack, int count) {
	while (count--) {
		if (stack->type == IASM_TYPE_OPERATOR || stack->type == IASM_TYPE_TYPE) {
			return 1;
		}
		stack++;
	}
	return 0;
}

void token_add(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb += *rb);
}

void token_sub(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb -= *rb);
}

void token_div(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(
		if (*rb == 0) {
			*lb = 0;
			return;
		}
		*lb /= *rb;
	);
}

void token_mul(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb *= *rb);
}

void token_mod(printer_token_t * left, printer_token_t * right) {
	if (left->type == FLOAT64 || right->type == FLOAT64) {
		*(uint64_t *) left->buffer = 0xffffffffffffffff;
		left->type = FLOAT64;
		return;
	}
	uint64_t * lb = (uint64_t *) left->buffer;
	uint64_t * rb = (uint64_t *) right->buffer;
	*lb ^= *rb;
	if (*rb == 0) {
		return;
	}
	*lb %= *rb;
}

void token_xor(printer_token_t * left, printer_token_t * right) {
	uint64_t * lb = (uint64_t *) left->buffer;
	uint64_t * rb = (uint64_t *) right->buffer;
	*lb ^= *rb;
}

void token_log_not(printer_token_t * operand) {
	operator_function_innard_single(*b = !*b);
}

void token_bit_not(printer_token_t * operand) {
	uint64_t * b = (uint64_t *) operand->buffer;
	*b = ~*b;
}

void token_log_and(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb = *lb && *rb);
}

void token_bit_and(printer_token_t * left, printer_token_t * right) {
	uint64_t * lb = (uint64_t *) left->buffer;
	uint64_t * rb = (uint64_t *) right->buffer;
	*lb = *lb & *rb;
}

void token_log_or(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb = *lb || *rb);
}

void token_bit_or(printer_token_t * left, printer_token_t * right) {
	uint64_t * lb = (uint64_t *) left->buffer;
	uint64_t * rb = (uint64_t *) right->buffer;
	*lb = *lb | *rb;
}

void token_pow(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb = powl((long double) *lb, (long double) *rb));
}

void token_cmp_ge(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb = *lb >= *rb);
}

void token_cmp_be(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb = *lb <= *rb);
}

void token_cmp_g(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb = *lb > *rb);
}

void token_cmp_b(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb = *lb < *rb);
}

void token_cmp_e(printer_token_t * left, printer_token_t * right) {
	operator_function_innard(*lb = *lb == *rb);
}

void token_shr(printer_token_t * left, printer_token_t * right) {
	uint64_t * lb = (uint64_t *) left->buffer;
	uint64_t * rb = (uint64_t *) right->buffer;
	*lb >>= *rb;
}

void token_shl(printer_token_t * left, printer_token_t * right) {
	uint64_t * lb = (uint64_t *) left->buffer;
	uint64_t * rb = (uint64_t *) right->buffer;
	*lb <<= *rb;
}

int generic_operator(printer_token_t ** sp, operator_evaluator_t evaluator, int * count) {
	if (*count < 2) {
		return -1;
	}
	printer_token_t * left = (*sp) - 2;
	printer_token_t * right = (*sp) - 1;
	evaluator(left, right);
	*count -= 1;
	(*sp) -= 1;
	memset(*sp, 0, sizeof(printer_token_t));
	return 0;
}

int generic_operator_single(printer_token_t ** sp, operator_evaluator_single_t evaluator, int * count) {
	if (*count < 1) {
		return -1;
	}
	printer_token_t * operand = (*sp) - 1;
	evaluator(operand);
	return 0;
}

int eval_operator(printer_token_t ** sp, printer_token_t * token, int * count) {
	int op = *(int *) token->buffer;
	switch (op) {
		case OPERATOR_ADD: return generic_operator(sp, token_add, count);
		case OPERATOR_SUB: return generic_operator(sp, token_sub, count);
		case OPERATOR_DIV: return generic_operator(sp, token_div, count);
		case OPERATOR_MUL: return generic_operator(sp, token_mul, count);
		case OPERATOR_MOD: return generic_operator(sp, token_mod, count);
		case OPERATOR_XOR: return generic_operator(sp, token_xor, count);
		case OPERATOR_LOG_NOT: return generic_operator_single(sp, token_log_not, count);
		case OPERATOR_BIT_NOT: return generic_operator_single(sp, token_bit_not, count);
		case OPERATOR_LOG_AND: return generic_operator(sp, token_log_and, count);
		case OPERATOR_BIT_AND: return generic_operator(sp, token_bit_and, count);
		case OPERATOR_LOG_OR: return generic_operator(sp, token_log_or, count);
		case OPERATOR_BIT_OR: return generic_operator(sp, token_bit_or, count);
		case OPERATOR_POW: return generic_operator(sp, token_pow, count);
		case OPERATOR_CMP_GE: return generic_operator(sp, token_cmp_ge, count);
		case OPERATOR_CMP_BE: return generic_operator(sp, token_cmp_be, count);
		case OPERATOR_CMP_G: return generic_operator(sp, token_cmp_g, count);
		case OPERATOR_CMP_B: return generic_operator(sp, token_cmp_b, count);
		case OPERATOR_CMP_E: return generic_operator(sp, token_cmp_e, count);
		case OPERATOR_SHR: return generic_operator(sp, token_shr, count);
		case OPERATOR_SHL: return generic_operator(sp, token_shl, count);
	}
	return -1;
}

int eval_stack(printer_token_t * _stack, printer_token_t * token, int count) {
	int i = 0;
	printer_token_t stack[MAX_TOKENS] = {0};
	printer_token_t * sp = stack;
	memset(stack, 0, sizeof(printer_token_t) * MAX_TOKENS);
	while (count--) {
		if (token->type == IASM_TYPE_OPERATOR) {
			int r = eval_operator(&sp, token, &i);
			if (r < 0) {
				return -1;
			}
			token++;
			continue;
		}
		if (token->type == IASM_TYPE_FUNCTION) {
			uint64_t r = safe_call_func(*(uint64_t *) token->buffer, sp, i, token->op_count);
			*(uint64_t *) token->buffer = r;
			token->type = INT64;
			token->associativity = ASSOCIATIVITY_LEFT;
			token->precedence = 2;
			token->op_count = 0;
			*sp++ = *token++;
			i++;
			continue;
		}
		if (token->type == IASM_TYPE_TYPE) {
			if (i < 1) {
				return -1;
			}
			printer_token_t * prev = sp - 1;
			prev->type = *(int *) token->buffer;
			token++;
			while (count--) {
				*sp++ = *token++;
				i++;
			}
			break;
		}
		*sp++ = *token++;
		i++;
	}
	memcpy(_stack, stack, sizeof(stack));
	return i;
}

int evaluate_print_expression(char * expression, int end) {
	printer_token_t output_buffer[MAX_TOKENS] = {0}; // this is really wasteful
	printer_token_t operators_buffer[MAX_TOKENS] = {0}; // this is really wasteful
	printer_token_t * output = output_buffer;
	printer_token_t * operators = operators_buffer;
	printer_state_t state = {
		.negative = 0,
	};
	int output_count = 0;
	int operator_count = 0;
	memset(output_buffer, 0, sizeof(printer_token_t) * MAX_TOKENS);
	memset(operators_buffer, 0, sizeof(printer_token_t) * MAX_TOKENS);
	while (*expression) {
		int consumed = read_token(expression, &state, &output, &operators, &output_count, &operator_count);
		if (consumed <= 0) {
			printf("error: expression evaluation failed");
			printf(end ? "\n" : ", ");
			return -1;
		}
		expression += consumed;
		expression = skip_whitespace(expression);
	}
	while (operator_count--) {
		*output++ = *--operators;
		output_count++;
	}

	int i = 128;
	while (output_count > 1 && operators_present(output_buffer, output_count) && i > 0) {
		int new_count = eval_stack(output_buffer, output_buffer, output_count);
		if (new_count <= 0) {
			printf("error: expression evaluation failed");
			printf(end ? "\n" : ", ");
			return -1;
		}
		output_count = new_count;
		i--;
	}

	if (i == 0) {
		printf("error: expression evaluation failed");
		printf(end ? "\n" : ", ");
		return -1;
	}

	output = output_buffer;
	print_typed_bytes(output->buffer, output->type, 8);
	printf(end ? "\n" : ", ");
	return 0;
}

void print_statement(char * arguments, int end) {
	// old operation
	char _buffer[1024] = {0};
	char _buffer2[1024] = {0};
	char _buffer3[1024] = {0};
	sscanf(arguments, "%s%s%s", _buffer, _buffer2, _buffer3);

	int swap = *_buffer2;
	int swap2 = *_buffer3;
	char * regname = swap ? _buffer2 : _buffer;
	char * _type = swap ? _buffer : _buffer2;
	char * _size = swap2 ? _buffer : _type;
	_type = swap2 ? _buffer2 : _type;
	regname = swap2 ? _buffer3 : regname;

	char buffer[4096];
	int size = 0;
	int type = 0;
	int reassignable = 0;
	if (resolve_any_register_or_label(regname, buffer, &size, &type, &reassignable)) {
		// try and evaluate as expresssion (new operation grafted ontop)
		evaluate_print_expression(skip_whitespace(arguments), end);
		return;
	}

	int type_identifier = *_type ? decode_type(_type) : type;
	int size_identifier = *_size ? decode_type(_size) : type;
	if (type_identifier == IASM_TYPE_NULL || size_identifier == IASM_TYPE_NULL) {
		evaluate_print_expression(skip_whitespace(arguments), end);
		return;
	}
	if (reassignable) {
		size = decode_type_size(size_identifier);
	}
	print_typed_bytes(buffer, type_identifier, size);
	printf(end ? "\n" : ", ");
	return;
}

int print_function(char * arguments) {
	char * statement = NULL;
	char * base = arguments;
	char c = *arguments;
	while (c) {
		c = *arguments;
		if (c == ',' || c == 0) {
			size_t length = arguments - base;
			statement = malloc(length + 1); // this is really dumb
			memcpy(statement, base, length);
			statement[length] = 0;
			print_statement(statement, c == 0);
			free(statement);
			base = arguments + 1;
		}
		arguments++;
	}
	return 1;
}
