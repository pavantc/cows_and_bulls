#include <stdio.h>

static void
print_num(int *narr)
{
	int num = 0, index = 1, i;
	for (i = 3; i >= 0; i--) {
		printf("%d", narr[i]);
	}
	printf("\n");
}

static void
get_cows_bulls(int *input, int *cows, int *bulls)
{
	printf("ncows: \n");
	scanf("%d", cows);
	printf("nbulls: \n");
	scanf("%d", bulls);

	if (*bulls == 4) {
		printf("Yay!\n");
	}
}

static void
display_guess(int *guess, int set_size)
{
	if (set_size == 1) {
		printf("Your number is ");
	} else {
		printf("\nMy guess: ");
	}
	print_num(guess);
}

#include "solve_cb.c"

int
main(int argc, char *argv[])
{
	int guess_count = 0;
	int ret;

	init_solve_cb();
	ret = solve_cb(&guess_count);
	if (ret) {
		printf("Incorrect feedback somewhere\n");
	}
	return (ret);
}
