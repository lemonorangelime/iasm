#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

uint64_t strtolh(const char * nptr, char ** endptr, int base) {
	register const char *s = nptr;
	register uint64_t acc;
	register int c;
	register uint64_t cutoff;
	register int neg = 0, any, cutlim;
	while (isspace(c)) { c = *s++; }
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+') {
		c = *s++;
	}
	if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0) {
		base = c == '0' ? 8 : 10;
	}
	cutoff = neg ? -(unsigned long long int) LONG_MIN : LONG_MAX;
	cutlim = cutoff % (unsigned long long int) base;
	cutoff /= (unsigned long long int) base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit(c)) {
			c -= '0';
		} else if (isalpha(c)) {
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		} else {
			break;
		}
		if (c >= base) {
			break;
		}
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
			any = -1;
		} else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LONG_MIN : LONG_MAX;
	} else if (neg) {
		acc = -acc;
	}
	if (endptr != 0) {
		*endptr = (char *) (any ? s - 1 : nptr);
	}
	return acc;
}

uint64_t strtolhauto(char * string) {
	if ((string[0] == u'0') && (string[1] == u'x')) {
		return strtolh(string, 0, 16);
	} else if ((string[0] == u'0') && (string[1] == u'b')) {
		return strtolh(string + 2, 0, 2);
	} else if (string[0] == u'#') {
		return strtolh(string + 1, 0, 16);
	} else if ((string[0] == u'0') && (string[1] == u'o')) {
		return strtolh(string + 2, 0, 8);
	} else if (string[strlen(string) - 1] == u'h') {
		return strtolh(string, 0, 16);
	} else {
		return strtolh(string, 0, 10);
	}
}
