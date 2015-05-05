#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <getopt.h>

#define NUM_DIGITS	4
#define	MAX_TRIES	64
#define	GIVE_UP_CODE	'x'
#define	SUMMARY_CODE	's'

const int reps_allowed = 1;

struct user_stats {
	uint8_t us_guess[NUM_DIGITS];
	int us_ncows;
	int us_nbulls;
};

static int
repeated_digit(uint8_t *narr, int len, int digit)
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
generate_number(uint8_t *narr)
{
	int c = 0;
	int k;
	int n;

	while (c < NUM_DIGITS) {
		/* Get digit */
		n = random() % 10;
		if (!repeated_digit(narr, c, n)) {
			narr[c++] = n;
		}
	}
}

static int
check_and_respond(uint8_t *input, uint8_t *ref, int *cows, int *bulls)
{
	int i, j;
	int ret;

	*cows = 0;
	*bulls = 0;
	for (i = 0; i < NUM_DIGITS; i++) {
		for (j = 0; j < NUM_DIGITS; j++) {
			if (input[i] == ref[j]) {
				if (i == j) {
					(*bulls)++;
				} else {
					(*cows)++;
				}
			}
		}
	}
	if (*bulls == NUM_DIGITS) {
		ret = 0;
	} else {
		printf("\n%d cow%s, %d bull%s\n", *cows, (*cows > 1 ? "s" : ""), *bulls, (*bulls > 1 ? "s" : ""));
		ret = 1;
	}
	return (ret);
}

static int
get_user_input(uint8_t *input)
{
	char buf[80];
	int good_input = 0;
	int i;

	memset(buf, 10, 80);
	while (!good_input) {
		fgets(buf, 79, stdin);
		if (tolower(buf[0]) == GIVE_UP_CODE) {
			return (GIVE_UP_CODE);
		}
		if (tolower(buf[0]) == SUMMARY_CODE) {
			return (SUMMARY_CODE);
		}
		for (i = 0; i < NUM_DIGITS; i++) {
			if (buf[i] < '0' || buf[i] > '9') {
				printf("Bad input. Enter again\n");
				break;
			} else {
				input[i] = buf[i] - '0';
			}
		}
		if (i == NUM_DIGITS) {
			int k;

			for (k = 0; k < NUM_DIGITS - 1; k++) {
				if (repeated_digit(&input[k+1], NUM_DIGITS - 1 - k, input[k])) {
					printf("Input has repeated digits. Bad input. Enter again\n");
					break;
				}
			}
			if (k == NUM_DIGITS - 1) {
				good_input = 1;
			}
		}
	}
	return (0);
}

static void
store_stats(struct user_stats *us, uint8_t *input, int tc, int cows, int bulls)
{
	memcpy(us[tc].us_guess, input, NUM_DIGITS);
	us[tc].us_ncows = cows;
	us[tc].us_nbulls = bulls;
}

static void
digits_to_str(uint8_t *input, char *nbuf)
{
	int k;

	for (k = 0; k < NUM_DIGITS; k++) {
		nbuf[k] = input[k] + '0';
	}
	nbuf[k] = '\0';
}

static void
print_stats(struct user_stats *us, int count)
{
	int i;
	char nbuf[NUM_DIGITS + 1] = {0};

	printf("%-8s%-8s%-8s\n", "GUESSES", "COWS", "BULLS");
	for (i = 0; i < count; i++) {
		digits_to_str(&us[i].us_guess[0], &nbuf[0]);
		printf("%-8s%-8d%-8d\n", nbuf, us[i].us_ncows, us[i].us_nbulls);
	}
}

static void
print_results(struct user_stats *us, int give_up, int try_count, char *nbuf)
{
	if (give_up) {
		if (try_count == 0) {
			printf("You did not even start!!\n");
		} else {
			printf("The number was %s. These were your guesses:\n", nbuf);
		}
	} else {
		printf("You cracked it! This is how you went about it:\n");
	}

	if (try_count) {
		print_stats(&us[0], try_count);
		if (give_up) {
			printf("You gave up at this point\n");
		}
	}
}

static void
describe_game()
{
	printf("The game is called \"COWS and BULLS\". The idea is to guess the %d digit number I have chosen.\n"
	    "Start by guessing any %d digit number (without repeating digits. I will tell you if you do anyway)\n"
	    "If a digit in your guess is present in my chosen number, it qualifies as a \"cow\".\n"
	    "Further, if that digit is also in the right position in my chosen number, it is a \"bull\".\n"
	    "NOTE: 0 is treated like any other digit and can occupy any position including the beginning of the number.\n"
	    "The game ends when you get %d bulls. You can enter 'x' at any point to give up, or 's' for a summary so far.\n"
	    "WARNING! This is quite an addictive game :)\n",
	    NUM_DIGITS, NUM_DIGITS, NUM_DIGITS);
}

#define	PROFILE_DIR	".cowsandbulls"
#define USERNAME_LEN	8

static int
create_profile_dir()
{
	return (mkdir(PROFILE_DIR, 0755));
}

static int
request_username(char *uname)
{
	char name_buf[80] = {0};
	int copylen;

	printf("Enter username (Press enter to skip, 1st character must be a letter from the alphabet):\n");
	fgets(&name_buf[0], 80, stdin);
	copylen = strlen(&name_buf[0]);
	copylen = (copylen > USERNAME_LEN ? USERNAME_LEN : copylen - 1);
	strncpy(&uname[0], &name_buf[0], copylen);
	return (isalpha(uname[0]));
}

static void
mk_profile_path(char username[], char path_buf[])
{
	char *path = &path_buf[0];
	int pdlen = sizeof (PROFILE_DIR);

	strncpy(path, PROFILE_DIR, pdlen - 1);
	strcat(path, "/");
	strncat(path, &username[0], USERNAME_LEN);
	strcat(path, "");
}

typedef struct saved_user_stats {
	int sus_num_completed_games;
	int sus_total_completed_try_count;
	int sus_num_given_up_games;
	int sus_total_given_up_try_count;
	int sus_total_games;
	int sus_max_given_up_try_count;
	int sus_max_completed_try_count;
} __attribute__((packed)) su_stats_t;

static void
print_saved_user_stats(FILE *fp, int summary)
{
	su_stats_t sstats, *s;
	uint8_t n[NUM_DIGITS];
	int try_count;
	struct user_stats us[MAX_TRIES];
	int total_games;
	int total_try_count;
	int avg_try_count;
	int avg_give_up_try_count = 0;
	int avg_completed_try_count = 0;

	fseek(fp, 0, SEEK_SET);
	fread(&sstats, sizeof (sstats), 1, fp);
	s = &sstats;

	total_games = s->sus_num_completed_games + s->sus_num_given_up_games;
	total_try_count = s->sus_total_completed_try_count + s->sus_total_given_up_try_count;
	avg_try_count = total_try_count / total_games;

	if (s->sus_num_given_up_games) {
		avg_give_up_try_count = s->sus_total_given_up_try_count / s->sus_num_given_up_games;
	}
	if (s->sus_num_completed_games) {
		avg_completed_try_count = s->sus_total_completed_try_count / s->sus_num_completed_games;
	}

	printf("Saved stats:\n");
	printf("Total games = %d\n", total_games);
	printf("Total try count = %d\n", total_try_count);
	printf("Avg try count = %d\n", avg_try_count);
	printf("Avg give up try count = %d\n", avg_give_up_try_count);
	printf("Avg completed try count = %d\n", avg_completed_try_count);
	printf("Max give up count = %d\n", s->sus_max_given_up_try_count);
	printf("Max completed count = %d\n", s->sus_max_completed_try_count);

	while (!summary && total_games) {
		if (fread(&n[0], sizeof (uint8_t), NUM_DIGITS, fp) < NUM_DIGITS) {
			fprintf(stderr, "Unexpected EOF while reading the number\n");
			return;
		}
		if (fread(&try_count, sizeof (try_count), 1, fp) < 1) {
			fprintf(stderr, "Unexpected EOF while reading try_count\n");
			return;
		}
		if (fread(&us[0], sizeof (struct user_stats), try_count, fp) < try_count) {
			fprintf(stderr, "Unexpected EOF while reading user_stats\n");
			return;
		}
		printf("Number: %d%d%d%d\n", n[0], n[1], n[2], n[3]);
		printf("Try count: %d\n", try_count);
		print_stats(&us[0], try_count);
		total_games--;
	}

}

static void
save_user_stats(struct user_stats *us, uint8_t *number, char username[], int give_up, int try_count)
{
	struct stat s;
	char profile_path[32];
	char *open_mode;
	int new_profile = 0;
	su_stats_t stats = {0};
	FILE *fp;

	mk_profile_path(&username[0], &profile_path[0]);

	if (lstat(&profile_path[0], &s) == 0) {
		/* Found a profile. Read existing stats from file */
		open_mode = "r+";
	} else {
		/* Need to create a profile */
		open_mode = "w+";
		new_profile = 1;
	}

	fp = fopen(&profile_path[0], open_mode);
	if (fp == NULL) {
		perror("fopen profile");
		return;
	}

	if (!new_profile) {
		fread(&stats, sizeof (stats), 1, fp);
	}

	if (give_up) {
		stats.sus_num_given_up_games++;
		stats.sus_total_given_up_try_count += try_count;
		if (try_count > stats.sus_max_given_up_try_count) {
			stats.sus_max_given_up_try_count = try_count;
		}
	} else {
		stats.sus_num_completed_games++;
		stats.sus_total_completed_try_count += try_count;
		if (try_count > stats.sus_max_completed_try_count) {
			stats.sus_max_completed_try_count = try_count;
		}
	}
	stats.sus_total_games = stats.sus_num_completed_games + stats.sus_num_given_up_games;

	/*
	 * Structure of stats file:
	 * su_stats_t
	 * number1
	 * try_count1
	 * user_stats[try_count1]
	 * number2
	 * try_count2
	 * user_stats[try_count2]
	 * ..
	 */
	fseek(fp, 0, SEEK_SET);
	fwrite(&stats, sizeof (stats), 1, fp);
	fseek(fp, 0, SEEK_END);
	fwrite(&number[0], sizeof (uint8_t), NUM_DIGITS, fp);
	fwrite(&try_count, sizeof (try_count), 1, fp);
	fwrite(us, sizeof (struct user_stats), try_count, fp);
	fflush(fp);
	print_saved_user_stats(fp, 1);
}

void
usage(int argc, char *argv[])
{
	fprintf(stderr, "Usage:\n%s\n", argv[0]);
	fprintf(stderr, "\tPlay the cows and bulls game\n");
	fprintf(stderr, "%s -s stats_file\n", argv[0]);
	fprintf(stderr, "\tPrint the statistics from the file \"stats_file\" and exit\n");
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	uint8_t number[NUM_DIGITS];
	uint8_t input[NUM_DIGITS];
	char rbuf[4];
	struct user_stats us[MAX_TRIES];
	int try_count;
	int cows, bulls;
	int done;
	char nbuf[NUM_DIGITS + 1] = {0};
	int give_up = 0;
	int code;
	char username[USERNAME_LEN + 1] = {0};
	int uname_provided = 0;
	char profile_path[32];
	char *stats_filename = NULL;
	FILE *stats_fp;
	int opt;

	while ((opt = getopt(argc, argv, "s:h")) != -1) {
		switch (opt) {
		case 'h':
			describe_game();
			usage(argc, argv);
		case 's':
			stats_filename = optarg;
			stats_fp = fopen(stats_filename, "r");
			if (stats_fp == NULL) {
				fprintf(stderr, "Could not open stats file %s\n", stats_filename);
				usage(argc, argv);
			} else {
				print_saved_user_stats(stats_fp, 0);
				exit(0);
			}
		default:
			usage(argc, argv);
		}
	}

	describe_game();
	create_profile_dir();
	uname_provided = request_username(&username[0]);
	if (uname_provided) {
		mk_profile_path(&username[0], &profile_path[0]);
		printf("Saving stats to %s\n", &profile_path[0]);
	}

	srandom(time(NULL));

	while (1) {
		done = 0;
		try_count = 0;
		give_up = 0;
		memset(&number[0], 10, 4);
		generate_number(&number[0]);

		digits_to_str(&number[0], &nbuf[0]);

		while (!done && (try_count < MAX_TRIES)) {
			printf("\nEnter number. Enter '%c' to give up, '%c' for summary so far\n", GIVE_UP_CODE, SUMMARY_CODE);
			code = get_user_input(&input[0]);
			if (code == GIVE_UP_CODE) {
				give_up = 1;
				break;
			}
			if (code == SUMMARY_CODE) {
				print_stats(&us[0], try_count);
				continue;
			}
			if (check_and_respond(&input[0], &number[0], &cows, &bulls) == 0) {
				done = 1;
			}
			store_stats(&us[0], &input[0], try_count, cows, bulls);
			try_count++;
		}

		print_results(&us[0], give_up, try_count, nbuf);

		if (uname_provided) {
			save_user_stats(&us[0], &number[0], &username[0], give_up, try_count);
		}

		printf("\nPlay another?\n");
		fgets(rbuf, 4, stdin);
		if (tolower(rbuf[0]) == 'y') {
			continue;
		} else {
			break;
		}
	}
	return (0);
}
