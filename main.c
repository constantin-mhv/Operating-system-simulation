#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int main(int argc, char const *argv[]) {
	int result, t_unit;
	char commands[MAX];
	/* open input file */
	FILE *in = fopen(argv[1], "rt");
	if (in == NULL) {
		printf("Can't open file %s\n", argv[1]);
		return -1;
	}
	/* open output file */
	FILE *out = fopen(argv[2], "wt");
	if (out == NULL) {
		printf("Can't open file %s\n", argv[2]);
		fclose(in);
		return -1;
	}
	/* read time quantum */
	fscanf(in, "%d\n", &t_unit);

	TMemory *M = alloc_TMemory(t_unit);
	if (!M) {
		fclose(in);
		fclose(out);
		return -1;
	}
	/* the commands are executed, in case of errors exit while loop */
	while (fgets(commands, MAX, in)){
		result = execute(M, commands, out);
		if (result == FALSE) {
			break;
		}
	}

	fclose(in);
	fclose(out);
	free_TMemory(M);
	if (result == FALSE) {
		printf("Memory allocation ERROR\n");
		return -1;
	}
	return 0;
}
