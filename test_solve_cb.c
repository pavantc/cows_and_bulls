#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

int verbose = 0;
int test_num[4];

/* Test number generation */
static int
used_digit(int *narr, int digit, int len)
{
	int k;

	for (k = 0; k < len; k++) {
		if (narr[k] == digit) {
			return (1);
		}
	}
	return (0);
}

static void
generate_number(int *narr)
{
	int c = 0;
	int d;

	while (c < 4) {
		/* Get digit */
		d = random() % 10;
		if (!used_digit(narr, d, c)) {
			narr[c++] = d;
		}
	}
}

static void
print_num(int *narr)
{
	int num = 0, index = 1, i;
	for (i = 3; i >= 0; i--) {
		printf("%d", narr[i]);
	}
	printf(" ");
}

/* Called by solve_cb() */
static void
get_cows_bulls(int *input, int *cows, int *bulls)
{
	int i, j;
	int ret;

	*cows = 0;
	*bulls = 0;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (input[i] == test_num[j]) {
				if (i == j) {
					(*bulls)++;
				} else {
					(*cows)++;
				}
			}
		}
	}
}

/* Called by solve_cb() */
static void
display_guess(int *guess, int set_size)
{
	if (verbose) {
		print_num(&guess[0]);
		printf("(ss:%d) ", set_size);
	}
}

static void
usage(int argc, char **argv)
{
	fprintf(stderr,"\nUsage:\n");
	fprintf(stderr, "%s [-n count] [-v]\nSpecify run count and verbosity\n", argv[0]);
	exit(0);
}

#include "solve_cb.c"

int
main(int argc, char *argv[])
{
	int i;
	int total_guess_count = 0, guess_count, max_guess = 0, min_guess = 100;
	float avg_guess_count;
	int run_count = 10;
	int opt;
	int ret;

	while ((opt = getopt(argc, argv, "n:v")) != -1) {
		switch (opt) {
		case 'n':
			run_count = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage(argc, argv);
			break;
		}
	}

	if (run_count == 0) {
		run_count = 10;
	}

	init_solve_cb();

	for (i = 0; i < run_count; i++) {
		generate_number(&test_num[0]);
		if (verbose) {
			print_num(&test_num[0]);
			printf(": ");
		}

		ret = solve_cb(&guess_count);

		if (ret) {
			printf("Failed to guess ");
			print_num(&test_num[0]);
			printf("\n");
			break;
		}

		if (verbose) {
			printf(" (%d guesses)\n", guess_count);
		}

		total_guess_count += guess_count;
		if (guess_count < min_guess) {
			min_guess = guess_count;
		}
		if (guess_count > max_guess) {
			max_guess = guess_count;
		}
	}

	avg_guess_count = (float)total_guess_count / run_count;
	printf("Avg guess count = %0.2f, min guess count = %d, max guess_count = %d\n", avg_guess_count, min_guess, max_guess);
	return (ret);
}
