// We want C99 functionality if it is available (it gives us long doubles)
#define _ISOC9X_SOURCE

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include "rpn.h"

#define VERSION "RPN version 0.1"

// -------------------------------------------------------------------------
// Misc. macros
// -------------------------------------------------------------------------

// A macro to make sure we have enough parameters on the stack
#define STACK_CHECK(n) \
	if(ssize < n) { \
		STRNCPY(output, "Not enough parameters", out_len); \
		return RPN_BAD_PARAM; \
	}

// This should be a large enough stack size for most operations
#define MAX_STACK_SIZE 20
#define MAX_TOKEN_SIZE 512

// FLOAT is the best floating-point type we can get
// (hopefully, HUGE_VALL should only be defined on C99 platforms, which should
// have all the functions listed below)
#ifdef HUGE_VALL
#  define FLOAT long double
#  define FLOOR floorl
#  define CEIL ceill
#  define FMOD fmodl
#  define POW powl
#  define SQRT sqrtl
#  define ABS fabsl
#  define FMA(x, y, z) ((x) * (y) + (z))
#  define FREXP frexpl
#  define LOG logl
#  define MODF modfl
#  define FLOAT_DIG LDBL_DIG
#else
#  define FLOAT double
#  define FLOOR floor
#  define CEIL ceil
#  define FMOD fmod
#  define POW pow
#  define SQRT sqrt
#  define ABS fabs
#  define FMA(x, y, z) ((x) * (y) + (z))
#  define FREXP frexp
#  define LOG log
#  define MODF modf
#  define FLOAT_DIG DBL_DIG
#endif

#  define TRUNC(x) (((x) >= 0.0) ? (FLOOR(x)) : (CEIL(x)))

// strncpy does not null-terminate
#define STRNCPY(dest, src, n) \
	do { \
		strncpy(dest, src, n); \
		dest[n] = '\0'; \
	} while(0)

// Check errno, and set buf to an error string and return if set
#define CHECK_ERRNO(buf, len, retval) \
	if(errno) { \
		STRNCPY(buf, strerror(errno), len); \
		return retval; \
	}

// -------------------------------------------------------------------------
// Utility functions
// -------------------------------------------------------------------------

// A modified verison of dave2548's power() function.
static FLOAT ipow(FLOAT number, int power) {
	FLOAT temp;
	
	if(power == 0.0) {
		return 1.0;
	} else if(power == 1.0) {
		return number;
	} else if(power < 0.0) {
		return 1.0/(ipow(number, -power));
	} else if(power % 2 == 0) {
		temp = ipow(number, power / 2);
		return(temp * temp);
	} else {
		temp = ipow(number, power / 2);
		return(temp * temp * number);
	}
}
	
// A helper macro for my_atof; take a digit, do error checking, and add it
// on to the mantissa
#define ADD_DIGIT(fl, dig, mult, coeff) \
	if((dig) <= (base)) { \
	   mantissa = FMA(mantissa, (mult), (dig) * (coeff)); \
	} else { \
		errno = EINVAL; \
		return 0.0; \
	}

// Given a string, create a FLOAT from it, using the given base.  We assume
// ASCII.  Sets errno on error.
static FLOAT my_atof(char * input, int base) {
	FLOAT mantissa = 0.0;
	long int exponent = 0;
	FLOAT multiplier = base;
	FLOAT coeff = 1.0;
	char c;

	// Loop through the string
	errno = 0;
	while(*input != '\0') {
		c = toupper(*input);
		if(isdigit(c)) {
			ADD_DIGIT(mantissa, c - '0', multiplier, 1.0/coeff);
		} else if(isupper(c)) {
			// Check for exponent
			if(c == 'E' && base < 'E' - 'A' + 10) {
				exponent = strtoul(input+1, NULL, base);
				if(errno) return 0.0;
				break;
			} else {
				ADD_DIGIT(mantissa, c - 'A' + 10, multiplier, 1.0/coeff);
			}
		} else if(c == '-') {
			mantissa = -mantissa;
		} else if(c == '.') {
			multiplier = 1.0;
		} else {
			errno = EINVAL;
			return 0.0;
		}
		++input;
		if(multiplier == 1.0) coeff *= base;
	}

	// There's probably a better way to do this
	return mantissa * ipow(base, exponent);
}

// A helper macro for my_ftoa; convert an integer into a digit (char),
// returning 0 if the value is invalid
#define INT_TO_DIGIT(x) \
	(((x) < 10) ? ((x) + '0') : (((x) < 36) ? ((x) + 'A' - 10) : (0)))

// A helper macro for my_ftoa; do error checking and put ch into buf
#define BUF_ADD(buf, end, ch) \
	if(buf < end) *buf++ = ch

// A helper macro for my_ftoa; given a digit, put it into buf
#define DO_DIGIT(buf, end, x) \
	do { \
		char c = INT_TO_DIGIT(x); \
		if(c) {\
			BUF_ADD(buf, end, c); \
		} else { \
			errno = EINVAL; \
			return; \
		} \
	} while(0)

// A helper macro for my_ftoa; terminate a string safely.
#define DO_TERMINATE(buf, end) \
	if(buf < end) { \
		*buf = 0; \
	} else { \
		*(buf-1) = 0; \
	}

// Given a FLOAT as input, create a string that contains the digits of the
// input value as they would appear in the given base.	Puts the exponent
// into exp as an integer.
static void my_ftoa(FLOAT input, int base, char *buf, size_t buflen, int *exp) {
	char *end = buf + buflen;
	char *start = buf;
	FLOAT frac;
	FLOAT whole;
	int j;
	int max_digits;
	
	// Check for negative value
	if(input < 0.0) {
		BUF_ADD(buf, end, '-');
		input = -input;
	}

	// Extract the exponent
	input = FREXP(input, exp);
	frac = *exp * LOG(2)/LOG(base);
	frac = MODF(frac, &whole);
	*exp = TRUNC(whole);
	input *= POW(base, frac);
	(*exp)++;

	// Find the number of digits we want
	max_digits = LOG(10)/LOG(base) * FLOAT_DIG;
	
	// Don't put a leading zero on the string
	input = MODF(input, &whole);
	if(TRUNC(whole) != 0) {
		DO_DIGIT(buf, end, TRUNC(whole));
	} else {
		--(*exp);
	}
	input *= base;

	// Iterate through the string and generate the digits
	for(j = 1; j < max_digits-1 && input > 0.0; ++j) {
		input = MODF(input, &whole);
		DO_DIGIT(buf, end, TRUNC(whole));
		input *= base;
	}
	DO_TERMINATE(buf, end);

	// Perform some special checks if the string is larger than we have
	// precision for
	if(j >= max_digits-1) {
		int i, j;
		char *ptr;
		char mbase;
		FLOAT x, y;

		// We want to round the last digit
		input = MODF(input, &whole);
		x = MODF(input*base, &y);
		if(y >= base/2 && whole < base-1) ++whole;
		DO_DIGIT(buf, end, TRUNC(whole));
		input *= base;
		DO_TERMINATE(buf, end);

		--buf;

		// Check for a number that can be rounded down
		for(j = 0; j < 2; ++j) {
			for(i = 0, ptr = buf-j; *ptr == '0' && ptr > start; --ptr) ++i;
			if(i >= 5) {
				for(ptr = buf-j; *ptr == '0' && ptr > start; --ptr) *ptr = '\0';
			}
		}

		// Find the max digit for this base (in ascii); we'll use this below
		mbase = INT_TO_DIGIT(base - 1);

		// Check for a number that can be rounded up
		for(j = 0; j < 5; ++j) {
			for(i = 0, ptr = buf-j; *ptr == mbase && ptr > start; --ptr) ++i;
			if(i >= 5) {
				for(ptr = buf-j; *ptr == mbase && ptr > start; --ptr) {
					*ptr = '\0';
				}
				++(*ptr);
				if(ptr == start) {
					if(*ptr > '9') {
						*ptr = *ptr - '0' + 'A' - 10;
						if(*exp >= 0) ++(*exp);
					}
					if(*ptr > mbase) *ptr = '1';
				}
			}
		}
	}
}

// Move memory from src to dest, being sure not to pass outside of buf.
static void rpn_mem_move(char * dest, char * src, int len, char * buf, int buf_len) {
	char *end = buf + buf_len;

	while(len > 0 && (dest < buf || src < buf)) {
		dest++; src++; len--;
	}

	while(dest + len > end || src + len > end) len--; // TODO: This is SLOW!

	if(len >= 0) memmove(dest, src, len);
}

// Insert a decimal at position loc
static void rpn_insert_decimal(char *str, int len, int str_len, int loc) {
	rpn_mem_move(str+loc+1, str+loc, str_len-loc+1, str, len);
	if(loc < len) str[loc] = '.';
}

// Convert a string to exponent notation
static void rpn_exponent_notation(char *str, int len, long int exponent, int str_len) {
	// We can't fit the whole string, so use exponent notation
	int loc = str_len + 1;
	if(loc < len && len > 2 && str[1] == '\0') {
		// Add a zero to the end of the number
		str[1] = '0';
		loc++;
	}
	if(loc > len-17) loc = len-17;
	rpn_insert_decimal(str, len, str_len, 1);
	snprintf(str+loc, 16, "e%ld", exponent-1);
}

// Create human-readable output from a floating-point number.
// Returns 0 on success, -1 on failure.
static int rpn_make_string(FLOAT input, char *str, int len, int base) {
	int exponent;
	int str_len, j;

	if(input < 0.0) len--;

	// Get the string in mantissa-exponent form
	my_ftoa(input, base, str, len, &exponent);
	CHECK_ERRNO(str, len, -1);

	// printf("%s %d\n", str, (int)exponent);
	str_len = strlen(str);
	if(len > 0 && str[0] == '-') {
	  str++;
	  str_len--;
	}

	// zero is a special case
	if(*str == '\0') {
		STRNCPY(str, "0", len);
		return 0;
	}

	if(exponent < str_len) {
		// The number includes all the information we need.
		if(exponent > 0) {
			// We must add a decimal point, since 
			rpn_insert_decimal(str, len, str_len, exponent);
		} else {
			// The number is much less than zero.
			if(str_len - exponent + 3 > len) {
				// We can't fit the whole string, so use exponent notation
				rpn_exponent_notation(str, len, exponent, str_len);
			} else {
				// We can just add zero's to the beginning of the string
				rpn_mem_move(str-exponent+2, str, str_len+1, str, len);
				if(0 < len) str[0] = '0';
				if(1 < len) str[1] = '.';
				for(j = 2; j < 2-exponent; ++j) {
					if(j < len) str[j] = '0';
				}
			}
		}
	} else /* if(exponent > str_len) */ {
		// The number is bigger than what the string represents
		if(exponent >= len) {
			// We can't fit the whole string, so use exponent notation
			rpn_exponent_notation(str, len, exponent, str_len);
		} else {
			// We can just add zero's to the end of the string
			for(j = str_len; j < exponent; ++j) {
				if(j < len) str[j] = '0';
			}
			if(exponent < len) str[exponent] = '\0';
		}
	}

	if(len-1 > 0) str[len-1] = '\0'; // just in case
	return 0;
}

// -------------------------------------------------------------------------
// Initialization and destruction
// -------------------------------------------------------------------------

void rpn_calc_close(void) {
}

void rpn_calc_init(void) {
}

// -------------------------------------------------------------------------
// The calculator
// -------------------------------------------------------------------------

static int rpn_calc_internal(
		const char *input,
		char *output,
		size_t out_len,
		FLOAT *stack) {

	char real_token[MAX_TOKEN_SIZE+1] = "0";
	char * const token = real_token + 1;
	char * new_token;
	int ssize = 0;
	int token_len = 0;
	int ibase = 10, obase = 10;
	int have_exponent = 0;

	for(;; ++input) {
		if(isdigit(*input) || *input == '.' ||
		   (ibase > 10 && ibase <= 16 &&
			toupper(*input) >= 'A' && toupper(*input) <= 'F') ||
		   (ibase > 16 && isupper(*input))) {
			// Still processing this token
			token[token_len++] = *input;
			if(*input != 0) continue;
		}

		if(*input == 'e' || *input == 'E') {
			// exponent
			token[token_len++] = *input;
			have_exponent = 1;
			continue;
		}

		if(have_exponent && *input == '-') {
			// negative exponent
			token[token_len++] = *input;
			continue;
		}

		if(token_len != 0) {
			// End of token
			if(ssize >= MAX_STACK_SIZE) {
				STRNCPY(output, "Stack full", out_len);
				return RPN_STACK_FULL;
			}
			token[token_len] = 0;

			// my_atof might not accept numbers that begin with a period.
			new_token = (token[0] == '.') ? real_token : token;

			stack[ssize++] = my_atof(new_token, ibase);
			CHECK_ERRNO(output, out_len, RPN_UNKNOWN);
			token_len = 0;
			have_exponent = 0;
		}

		// Check for end of string
		if(*input == 0) break;
		
		// Operation
		switch(*input) {
			case '+':
				// Add
				STACK_CHECK(2);
				stack[ssize-2] += stack[ssize-1];
				ssize--;
				break;
				
			case '-':
				// Subtract
				STACK_CHECK(2);
				stack[ssize-2] -= stack[ssize-1];
				ssize--;
				break;

			case '*':
				// Multiply
				STACK_CHECK(2);
				stack[ssize-2] *= stack[ssize-1];
				ssize--;
				break;
				
			case '/':
				// Divide
				STACK_CHECK(2);
				if(stack[ssize-1] == 0.0) {
					STRNCPY(output, "Divide by zero", out_len);
					return RPN_DIVIDE_BY_ZERO;
				}
				stack[ssize-2] /= stack[ssize-1];
				ssize--;
				break;

			case '%':
				// Modulus
				STACK_CHECK(2);
				if(stack[ssize-1] == 0.0) {
					STRNCPY(output, "Divide by zero", out_len);
					return RPN_DIVIDE_BY_ZERO;
				}
				stack[ssize-2] = FMOD(stack[ssize-2], stack[ssize-1]);
				ssize--;
				break;

			case '~':
				// Negate
				STACK_CHECK(1);
				stack[ssize-1] = -stack[ssize-1];
				break;

			case '^':
				STACK_CHECK(2);
				stack[ssize-2] = POW(stack[ssize-2], stack[ssize-1]);
				ssize--;
				break;

			case 'r':
				// Swap
				STACK_CHECK(2);
				{
					FLOAT temp = stack[ssize-1];
					stack[ssize-1] = stack[ssize-2];
					stack[ssize-2] = temp;
				}
				break;

			case 'v':
				// Square root
				STACK_CHECK(1);
				if(stack[ssize-1] < 0.0) {
					STRNCPY(output, "Cannot take sqrt of a negative",
						out_len);
					return RPN_ILLEGAL;
				}
				stack[ssize-1] = SQRT(stack[ssize-1]);
				break;

			case '|':
				// Absolute value
				STACK_CHECK(1);
				stack[ssize-1] = ABS(stack[ssize-1]);
				break;

			case 'i':
				// Set input base
				STACK_CHECK(1);
				ibase = FLOOR(stack[--ssize]);
				if(ibase < 2 || ibase > 36) {
					STRNCPY(output, "Invalid base", out_len);
					return RPN_ILLEGAL;
				}
				break;

			case 'o':
				// Set output base
				STACK_CHECK(1);
				obase = FLOOR(stack[--ssize]);
				if(obase < 2 || obase > 36) {
					STRNCPY(output, "Invalid base", out_len);
					return RPN_ILLEGAL;
				}
				break;

			case ' ':
			case '\t':
			case '\r':
			case '\n':
				// Ignore whitespace
				break;

			default:
				STRNCPY(output, "Illegal operation", out_len);
				return RPN_ILLEGAL;
		}
	}

	// Copy out the answer
	if(ssize == 0) {
		STRNCPY(output, "Empty stack", out_len);
		return RPN_EMPTY_STACK;
	} else if(ssize == 1) {
		if(rpn_make_string(stack[0], output, out_len, obase) == -1) {
			return RPN_UNKNOWN;
		}
	} else {
		STRNCPY(output, "Stack not empty", out_len);
		return RPN_NONEMPTY_STACK;
	}
	
	return RPN_OK;
}

int rpn_calc(const char *input, char *output, size_t out_len) {
	FLOAT stack[MAX_STACK_SIZE];

	// strcasecmp won't work right with _ISOC9X_SOURCE defined.
	if(!strcmp(input, "lag")) {
		STRNCPY(output, "This is Efnet.	 What do you expect?", out_len);
		return RPN_OK;
	} else if(!strcmp(input, "help")) {
		STRNCPY(output,
			"Valid operations: "
			"+ (add), "
			"- (subtract), "
			"* (multiply), "
			"/ (divide), "
			"~ (negate), "
			"% (modulus), "
			"^ (exponent), "
			"r (swap), "
			"v (sqrt), "
			"| (abs), "
			"i (set input base), "
			"o (set output base); "
			"bases higher than 16 require upper-case input",
			out_len);
		return RPN_OK;
	} else if(!strcmp(input, "version")) {
		STRNCPY(output, VERSION, out_len);
		return RPN_OK;
	}

	// Do the calculation
	return rpn_calc_internal(input, output, out_len, stack);
}

