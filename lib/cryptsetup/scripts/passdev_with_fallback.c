/*
 * passdev_with_fallback.c - tries to read a keyfile from a device
 *		but falls back to prompting the user for a password after
 *  	a timeout.
 *
 * Copyright (C) 2023   Lukas Prediger <lumip@lumip.de>
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */


#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 1

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * arg string consists of
 * - a leading delimiter character used to separate the argument to passdev from the prompt for askpass
 * - the argument string for passdev
 * - the delimiter
 * - the argument for askpass
 * 
 * The argument for askpass may be left empty, in which case the final delimiter can be omitted as well.
 * Examples:
 * - _/dev/sda1:keys/keyfile:5_password:
 * - _/dev/sda1:keys/keyfile
 */
static void
split_argument(char *arg, char **passdev_arg, char **askpass_arg)
{
	char *str;
	char split_char;

	split_char = *arg;
	arg++;

	*passdev_arg = arg;
	str = strchrnul(arg, split_char);
	if (*str == split_char) {
		*str = '\0';
		str++;
	}
	*askpass_arg = str;
}

static bool
spawn_process(const char *process, const char *arg)
{
	__pid_t pid;
	int status;

	pid = fork();
	if (pid == -1) {
		fprintf(stderr, "Could not fork subprocess\n");
		return false;
	}

	if (!pid) {
		execl(process, process, arg, NULL);
		fprintf(stderr, "Could not spawn subprocess %s\n", process);
		exit(EXIT_FAILURE);
	} else {
		if (waitpid(pid, &status, 0) < 0) {
			fprintf(stderr, "Could not wait for subprocess\n");
			return false;
		}
		if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
			return true;
		}
	}
	return false;
}

int
main(int argc, char **argv, char **envp)
{
	char *passdev_arg;
	char *askpass_arg;
	__pid_t passdev_pid;
	int passdev_status;

	/* We only take one argument */
	if (argc != 2) {
		fprintf(stderr, "Incorrect number of arguments\n");
		exit(EXIT_FAILURE);
	}

	split_argument(argv[1], &passdev_arg, &askpass_arg);

	if (spawn_process("/lib/cryptsetup/scripts/passdev", passdev_arg)) {
		exit(EXIT_SUCCESS);
	}

	fprintf(stderr, "passdev did not succeed\n");
	if (spawn_process("/lib/cryptsetup/askpass", askpass_arg)) {
		exit(EXIT_SUCCESS);
	}

	exit(EXIT_FAILURE);
}
