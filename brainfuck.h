/**
 * @author wwotz
 * @description BrainF*ck Compiler. Turns BrainF*ck code into C code.
 */

#ifndef BF_COMPILER_H_
#define BF_COMPILER_H_

typedef char bf_char_t;

/**
 * @description Return codes for bf functions.
 */
typedef enum bf_return_t {
	BF_NO_ERROR = 0,
	BF_MEMORY_ERROR,
} bf_return_t;

typedef enum bf_loc_t {
	BF_LOC_DISK,
	BF_LOC_MEMORY,
} bf_loc_t;

/**
 * @description Bf tokens found in bf programs.
 */
typedef enum BF_TOKEN_TYPE {
	BF_LEFT_ANGLE_TK,
	BF_RIGHT_ANGLE_TK,
	BF_PLUS_TK,
	BF_MINUS_TK,
	BF_PERIOD_TK,
	BF_COMMA_TK,
	BF_LEFT_SQUARE_BRACKET_TK,
	BF_RIGHT_SQUARE_BRACKET_TK,
	BF_TOKEN_TYPE_COUNT
} BF_TOKEN_TYPE;

/**
 * @description The BrainF*ck Lexer
 */
typedef struct bf_lexer_t {
	size_t lookahead;
	bf_char_t *source;
} bf_lexer_t;

typedef struct bf_compiler_t {
	bf_lexer_t *lexer;
	bf_char_t *csource;
} bf_compiler_t;

BFDEF bf_compiler_t *
bf_compiler_create(bf_loc_t loc, bf_char_t *data);

BFDEF void
bf_compiler_free(bf_compiler_t *compiler);

#ifdef BF_IMPLEMENTATION

static bf_lexer_t *
bf_lexer_create(bf_loc_t loc, bf_char_t *data);

static void
bf_lexer_free(bf_lexer_t *lexer);

#endif /* BF_IMPLEMENTATION */
#endif /* BF_COMPILER_H_ */
