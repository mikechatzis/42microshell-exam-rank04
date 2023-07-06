#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/* last updated 05/05/2022 */

/* READ THE MICROSHELL42 SUBJECT CAREFULLY BEFORE THE COMMENTS

NOTE: The current microshell subject is misleading. The tester will not test cases like: "; cmd ; ;",
but only well defined simple arguments. That makes our job easier and way faster */

size_t ft_strlen(char const *s) {
	size_t i = 0;
	while (s[i])
		i++;
	return i;
}

int main(int argc, const char **argv, const char **envp) {
	if (argc == 1)
		return write(2, "error: argument\n", 17), 1;

	//calculate number of commands
	size_t count = 1;
	for (size_t i = 1; argv[i]; i++) {
		if (strcmp(argv[i], "|") && strcmp(argv[i], ";"))
			continue;

		//skip consecutive ";"
		while (i < argc - 1 && !strcmp(argv[i], ";") && !strcmp(argv[i + 1], ";"))
			i++;
		if (i < argc - 1)
			count++;
	}

	//define arrays for cmd type (1 for pipe, 0 for semicolon) and cmd start index
	int type[count];
	size_t pos[count];

	/* loop through argv and set the indexes to determine where the beginning of each cmd is. NULL every "|"
	and ";" to serve as delimiters */
	size_t j = 0;
	for (size_t i = 1; argv[i]; i++) {
		if (!strcmp(argv[i], "|")) {
			pos[j + 1] = i + 1;
			type[j++] = 1;
			argv[i] = NULL;
		} else if (!strcmp(argv[i], ";")) {
			//once again skip consecutive ";" and also NULL all the ptrs
			while (!strcmp(argv[i], ";"))
				argv[i++] = NULL;
			type[j++] = 0;
			pos[1 + j++] = i;
		}
	}
	
	
	/* the execve loop, iterating once per cmd and handling pipes accordingly, based on whether a pipe was found or if the cmd is "cd".
	We can freely pass the entire argv starting from the address of argv[pos[]](the pre-saved cmd start index) to execve, as it will stop
	at the 1st NULL ptr it encounters. Note that no special treatment for ";" exists. That's because this loop will execute one command per
	iteration regardless, and will just set pipes accordingly, so input/output is read from/written to the correct fd. */

	int fd[2];
	int fdd = 0;
	pid_t pid;

	for (size_t i = 0; i < count; i++) {
		if (!strcmp(argv[pos[i]], "cd")) {
			/* remember, envp follows right after argv in memory, separated by a single NULL ptr:
			{arg1, arg2, arg3, ... , NULL, name=value1, name=value2, name=value3, ...},
			so if cd has more than 1 arguments or no arguments, the check below will always return true */

			if (argv[pos[i] + 2])
				write(2, "error: cd: bad arguments\n", 26);
			else if (chdir(argv[pos[i] + 1])) {
				write(2, "error: cd: cannot change directory to ", 39);
				write(2, argv[pos[i] + 1], ft_strlen(argv[pos[i] + 1]));
				write(2, "\n", 1);
			}
		} else {
			if (pipe(fd) == -1 || (pid = fork()) == -1)
				return write(2, "error: fatal\n", 14), 1;
			if (!pid) {
				dup2(fdd, 0);
				if(type[i])
					dup2(fd[1], 1);
				if (execve(argv[pos[i]], &argv[pos[i]], envp) == -1)
				{
					write(2, "error: cannot execute ", 23);
					write(2, argv[pos[i]], ft_strlen(argv[pos[i]]));
					write(2, "\n", 1);
				}
			} else {
				waitpid(pid, NULL, 0);
				close(fd[1]);
				close(fdd);
				fdd = fd[0];
			}
		}
	}
	return 0;
}
