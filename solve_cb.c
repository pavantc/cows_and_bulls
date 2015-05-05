/*
 * Program to solve the 4-digit cows and bulls game with no repetitions
 * Logic:
 * 1. Create an initial set of all possible 4-digit permutations
 * 2. Guess a number (a random number from the solution set)
 * 3. From the user, get the number of cows and bulls in the number chosen in 2. If number of bulls is 4, exit.
 * 4. Create a new solution set by picking numbers in the solution set that match the user feedback in 3.
 * 5. Go back to step 2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int
match(int *input, int *ref, int nc, int nb)
{
	int i, j;
	int cows = 0, bulls = 0;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (input[i] == ref[j]) {
				if (i == j) {
					bulls++;
				} else {
					cows++;
				}
			}
		}
	}
	if ((bulls == nb) && (cows == nc)) {
		/* Match */
		return (1);
	} else {
		return (0);
	}
}

int
generate_new_solution_set(int (*old_ss)[10000][4], int old_set_size, int (*new_ss)[10000][4], int *guess, int ncows, int nbulls)
{
	int new_set_size = 0;
	int i, k;

	for (i = 0; i < old_set_size; i++) {
		if (match(&(*old_ss)[i][0], guess, ncows, nbulls)) {
			for (k = 0; k < 4; k++) {
				(*new_ss)[new_set_size][k] = (*old_ss)[i][k];
			}
			new_set_size++;
		}
	}
	return (new_set_size);
}

void
guess_a_number(int (*ss)[10000][4], int set_size, int *guess)
{
	int i = 0, rindex = random() % set_size;
	for (i = 0; i < 4; i++) {
		guess[i] = (*ss)[rindex][i];
	}
}

int
repeated_digits(int number, int *digit_arr)
{
	int digits[10] = {0, };
	int d, k = 0;
	int num = number;

	while(num) {
		d = num % 10;
		if (digits[d] != 0) {
			/* Repeat digit */
			return (1);
		}
		/* Mark digit as present */
		digits[d] = 1;
		digit_arr[k++] = d;
		num = num/10;
	}

	/* If it is a 3 digit number, add 0 to the beginning */
	if (number < 1000) {
		if (digits[0] == 1) {
			return (1);
		}
		digit_arr[k++] = 0;
	}

	return (0);
}

int
generate_initial_solution_set(int init_ss[][4])
{
	int i, j;
	int count = 0;

	for (i = 100, j = 0; i < 10000; i++) {
		if (repeated_digits(i, &init_ss[j][0])) {
			continue;
		}
		j++;
		count++;
	}

	return (count);
}

void
init_solve_cb()
{
	srandom(time(NULL));
}

int
solve_cb(int *guess_count)
{
	int i;
	int set_size;
	int ss1[10000][4] = {{0,},};
	int ss2[10000][4] = {{0,},};
	int ncows, nbulls;
	int (*old_ss)[10000][4], (*new_ss)[10000][4], (*temp)[10000][4];
	int guess[4];
	int gc = 0;
	int ret;

	set_size = generate_initial_solution_set(ss1);
	old_ss = &ss1;
	new_ss = &ss2;

	while (1) {
		guess_a_number(old_ss, set_size, &guess[0]);
		gc++;
		display_guess(&guess[0], set_size);
		if (set_size == 1) {
			ret = 0;
			break;
		}
		get_cows_bulls(&guess[0], &ncows, &nbulls);
		if (nbulls == 4) {
			ret = 0;
			break;
		}

		set_size = generate_new_solution_set(&old_ss[0], set_size, &new_ss[0], &guess[0], ncows, nbulls);

		if (set_size == 0) {
			ret = 1;
			break;
		}

		/* Ping pong usage of old and new ss */
		temp = old_ss;
		old_ss = new_ss;
		new_ss = old_ss;
	}

	*guess_count = gc;
	return (ret);
}
