#include <stdio.h>

// nice name

enum {
	STATE_TEXT,
	STATE_QUOTE,
};

int comment_strip_text(char * q, char * statement) {
	switch (*statement) {
		case ';': *statement = 0; return STATE_TEXT;
		case '\'': *q = '\''; return STATE_QUOTE;
		case '"': *q = '"'; return STATE_QUOTE;
	}
	return STATE_TEXT;
}

int comment_strip_quote(int * skip, char * q, char * statement) {
	if (*skip != 0) {
		*skip -= 1;
		return STATE_QUOTE;
	}
	if (*statement == '\\') {
		*skip = 1;
		return STATE_QUOTE;
	}
	if (*statement == *q) {
		return STATE_TEXT;
	}
	return STATE_QUOTE;
}

int comment_strip_machine(int state, int * skip, char * q, char * statement) {
	switch (state) {
		case STATE_TEXT: return comment_strip_text(q, statement);
		case STATE_QUOTE: return comment_strip_quote(skip, q, statement);
	}
	return state;
}

void strip_whitespace(int state, char * start, char * statement) {
	if (state != STATE_TEXT) { return; }
	statement -= 1;
	if (start >= statement) { return; }

	while (start < statement) {
		if (*statement != ' ') {
			statement++;
			break;
		}
		statement--;
	}
	*statement = 0;
}


void comment_strip(char * statement) {
	int state = STATE_TEXT;
	int skip = 0;
	char quote = 0;
	char * start = statement;
	while (*statement) {
		state = comment_strip_machine(state, &skip, &quote, statement);
		if (*statement == 0) {
			strip_whitespace(state, start, statement);
			return;
		}
		statement++;
	}
	strip_whitespace(state, start, statement);
}
