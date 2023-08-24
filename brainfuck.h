/**
 * @author wwotz
 * @description BrainF*ck Compiler. Turns BrainF*ck code into C code.
 */

#ifndef BF_COMPILER_H_
#define BF_COMPILER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef BFSTATIC
#define BFDEF static
#else /* !defined(BFSTATIC) */
#define BFDEF extern
#endif /* BFDEF */

#if !defined(BF_MALLOC) || !defined(BF_REALLOC) || !defined(BF_FREE)
#define BF_MALLOC(sz) malloc(sz)
#define BF_REALLOC(x, newsz) realloc(x, newsz)
#define BF_FREE(x) free(x)
#endif

typedef char bf_char_t;

/**
 * @description Return codes for bf functions.
 */
typedef enum bf_return_t {
	BF_NO_ERROR = 0,
	BF_MEMORY_ERROR,
} bf_return_t;

typedef enum bf_bool_t {
	BF_TRUE = 1,
	BF_FALSE = 0,
} bf_bool_t;

typedef enum bf_loc_t {
	BF_LOC_DISK,
	BF_LOC_MEMORY,
} bf_loc_t;

/**
 * @description Bf tokens found in bf programs.
 */
typedef enum bf_token_t {
	BF_LEFT_ANGLE_TK,
	BF_RIGHT_ANGLE_TK,
	BF_PLUS_TK,
	BF_MINUS_TK,
	BF_PERIOD_TK,
	BF_COMMA_TK,
	BF_LEFT_SQUARE_BRACKET_TK,
	BF_RIGHT_SQUARE_BRACKET_TK,
	BF_EOF_TK,
	BF_TOKEN_TYPE_COUNT
} bf_token_t;

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

static bf_char_t *
bf_fmap(bf_char_t *fpath)
{
	FILE *fd;
	long fsize;
	bf_char_t *buffer;
	size_t nread, total;

	fd = fopen(fpath, "r");
	if (!fd) {
		return NULL;
	}

	fseek(fd, 0L, SEEK_END);
	fsize = ftell(fd);
	fseek(fd, 0L, SEEK_SET);

	buffer = BF_MALLOC((fsize + 1) * sizeof(*buffer));
	if (!buffer) {
		goto cleanup;
	}

	total = 0;
	while ((nread = fread(buffer, sizeof(*buffer), fsize, fd)) > 0) {
		total += nread;
	}

	buffer[total] = '\0';

	if (total != fsize || ferror(fd)) {
		BF_FREE(buffer);
		buffer = NULL;
	}
 cleanup:
	fclose(fd);
	return buffer;
}

static bf_lexer_t *
bf_lexer_create(bf_loc_t loc, bf_char_t *data)
{
	bf_lexer_t *lexer;
	assert(loc == BF_LOC_DISK || loc == BF_LOC_MEMORY);
	assert(data != NULL);
	lexer = BF_MALLOC(sizeof(*lexer));
	if (!lexer) {
		return NULL;
	}

	switch (loc) {
	case BF_LOC_DISK:
		lexer->source = bf_fmap(data);
		if (!lexer->source) {
			BF_FREE(lexer);
			return NULL;
		}
		break;
	case BF_LOC_MEMORY:
		lexer->source = strdup(data);
		if (!lexer->source) {
			BF_FREE(lexer);
			return NULL;
		}
	}

	lexer->lookahead = 0;
	return lexer;
}

static bf_bool_t 
bf_token_is_valid(bf_char_t lexeme)
{
	return (lexeme == '>' || lexeme == '<' || lexeme == '+'
		|| lexeme == '-' || lexeme == '.' || lexeme == ','
		|| lexeme == '[' || lexeme == ']' || lexeme == '\0');
}

static bf_token_t
bf_lexeme_to_token(bf_char_t lexeme)
{
	switch (lexeme) {
	case '>':
		return BF_RIGHT_ANGLE_TK;
	case '<':
		return BF_LEFT_ANGLE_TK;
	case '+':
		return BF_PLUS_TK;
	case '-':
		return BF_MINUS_TK;
	case '.':
		return BF_PERIOD_TK;
	case ',':
		return BF_COMMA_TK;
	case '[':
		return BF_LEFT_SQUARE_BRACKET_TK;
	case ']':
		return BF_RIGHT_SQUARE_BRACKET_TK;
	case '\0':
		return BF_EOF_TK;
	};
}

static bf_token_t
bf_lexer_next_token(bf_lexer_t *lexer)
{
	bf_token_t token;
	while (!bf_token_is_valid(lexer->source[lexer->lookahead])) {
		lexer->lookahead++;
	}
	
	token = bf_lexeme_to_token(lexer->source[lexer->lookahead]);
	if (token != BF_EOF_TK) {
		lexer->lookahead++;
	}
	return token;
}

static void
bf_lexer_free(bf_lexer_t *lexer)
{
	free(lexer->source);
	lexer->source = NULL;
	lexer->lookahead = 0;
	BF_FREE(lexer);
}

BFDEF bf_compiler_t *
bf_compiler_create(bf_loc_t loc, bf_char_t *data)
{
	bf_compiler_t *compiler;
	
	compiler = BF_MALLOC(sizeof(*compiler));
	if (!compiler) {
		return NULL;
	}

	compiler->lexer = bf_lexer_create(loc, data);
	if (!compiler->lexer) {
		BF_FREE(compiler);
		return NULL;
	}

	compiler->csource = NULL;
	return compiler;
}

BFDEF void
bf_compiler_exec(bf_compiler_t *compiler, const char *fpath)
{
	FILE *fd;
	int tabs = 0;
	bf_token_t token;
	assert(compiler && fpath);

	fd = fopen(fpath, "r");
	if (!fd) {
		fprintf(stderr, "Failed to open '%s'\n", fpath);
		return;
	}
	
	fprintf(fd, "#include <stdio.h>\n");
	fprintf(fd, "int\nmain()\n{\n");
	tabs++;
	while ((token = bf_lexer_next_token(compiler->lexer)) != BF_EOF_TK) {
		switch (token) {
		case BF_RIGHT_ANGLE_TK:
			for (int i = 0; i < tabs; i++) {
				fprintf(fd, "\t");
			}
			fprintf(fd, "p++;\n");
			break;
		case BF_LEFT_ANGLE_TK:
			for (int i = 0; i < tabs; i++) {
				fprintf(fd, "\t");
			}
			fprintf(fd, "p--;\n");
			break;
		case BF_PLUS_TK:
			for (int i = 0; i < tabs; i++) {
				fprintf(fd, "\t");
			}
			fprintf(fd, "buffer[p]++;\n");
			break;
		case BF_MINUS_TK:
			for (int i = 0; i < tabs; i++) {
				fprintf(fd, "\t");
			}
			fprintf(fd, "buffer[p]--;\n");
			break;
		case BF_PERIOD_TK:
			for (int i = 0; i < tabs; i++) {
				fprintf(fd, "\t");
			}
			fprintf(fd, "printf(\"%%c\", buffer[p]);\n");
			break;
		case BF_COMMA_TK:
			for (int i = 0; i < tabs; i++) {
				fprintf(fd, "\t");
			}
			fprintf(fd, "scanf(\"%%c\", buffer+p);\n");
			break;
		case BF_LEFT_SQUARE_BRACKET_TK:
			for (int i = 0; i < tabs; i++) {
				fprintf(fd, "\t");
			}
			fprintf(fd, "for (int i = 0; i < buffer[p]; i++) {\n");
			tabs++;
			break;
		case BF_RIGHT_SQUARE_BRACKET_TK:
			tabs--;
			for (int i = 0; i < tabs; i++) {
				fprintf(fd, "\t");
			}
			fprintf(fd, "}\n");
			break;
		}
	}
	fprintf(fd, "}\n");
	fclose(fd);
}

BFDEF void
bf_compiler_free(bf_compiler_t *compiler)
{
	bf_lexer_free(compiler->lexer);
	BF_FREE(compiler);
}

#endif /* BF_IMPLEMENTATION */
#endif /* BF_COMPILER_H_ */
