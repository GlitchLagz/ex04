#include "microshell.h"

int err(char *s)
{
	while(*s)
		write(2, s++, 1);
	return 1;
}

int cd(char **argv, int i)
{
	if (i != 2)
		return err("error: cd: bad arguments\n");
	else if (chdir(argv[1]) == -1)
		return err("error: cd: cannot change directory to "), err(argv[1]), err("\n");
	return 1;
}

int exec(char **argv, char **env, int i)
{
	int fd[2], status, pcheck = argv[i] && !strcmp(argv[i], "|");
	if (pcheck && pipe(fd) == -1)
		return err("error: fatal\n");
	int pid = fork();
	if (!pid)
	{
		argv[i] = 0;
		if (pcheck && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
			return err("error: fatal\n");
		execve(*argv, argv, env);
		return err("error: cannot execute "), err(*argv), err("\n");
	}
	waitpid(pid, &status, 0);
	if (pcheck && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		return err("error: fatal\n");
	return WIFEXITED(status) && WEXITSTATUS(status);
}

int main (int argc, char **argv, char **env)
{
	int i = 0, status = 0;
	if (argc > 1)
	{
		while (argv[i] && argv[++i])
		{
			argv += i;
			i = 0;
			while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
				i++;
			if (!strcmp(*argv, "cd"))
				status = cd(argv, i);
			else if (i)
				status = exec(argv, env, i);
		}
	}
	return status;
}
