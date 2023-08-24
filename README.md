# BrainF*ck - An Esoteric Programming Language

This is essentially a transpiler, it will transpile BrainF*ck code into C code, which can then
be compiled into machine code using `gcc`.

## How To
The `brainfuck.h` header file allows you to import the code and make use of the brainf*ck compiler
in a personal project.

To use it, we can for example write the following:

```c
#define BF_IMPLEMENTATION
#include "brainfuck.h"

int
main(int argc, char **argv)
{
	bf_compiler_t *compiler;
	compiler = bf_compiler_create(BF_LOC_DISK, "hello-world.bf");
	if (!compiler) {
		fprintf(stderr, "failed to create bf compielr!\n");
		exit(EXIT_FAILURE);
	}

	bf_compiler_exec(compiler, "hello-world.c");
	bf_compiler_free(compiler);
	return 0;
}
```
Where `hello-world.bf` is a file containing BrainF*ck code that prints: ```Hello World!```.
