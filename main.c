#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* Read the profile file 
 @@args "etc/profile"
 */
void read_profile(const char *name)
{
	int fd;
	int xflag_set = 0;
	int vflag_set = 0;

	INTOFF;
	if ((fd = open(name, O_RDONLY)) >= 0)
		setinputfd(fd, 1);
	INTON;
	if (fd < 0)
		return;
	/* -q turns off -x and -v just when executing init files */
	if (qflag)  {
	    if (xflag)
		    xflag = 0, xflag_set = 1;
	    if (vflag)
		    vflag = 0, vflag_set = 1;
	}
	cmdloop(0);
	if (qflag)  {
	    if (xflag_set)
		    xflag = 1;
	    if (vflag_set)
		    vflag = 1;
	}
	popfile();
}



/**
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
void main(int argc, char **argv)
{
  // Load config files, if any.

  new_sh_loop();

  // Perform any shutdown/cleanup.

//  return EXIT_SUCCESS;
}

/*
  Function Declarations for builtin shell commands:
 */
int new_sh_cd(char **args);
int new_sh_help(char **args);
int new_sh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &new_sh_cd,
  &new_sh_help,
  &new_sh_exit
};

int new_sh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

int new_sh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "new_sh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("new_sh");
    }
  }
  return 1;
}


int new_sh_exit(char **args)
{
  return 0;
}

int new_sh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("new_sh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("new_sh");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


int new_sh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < new_sh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return new_sh_launch(args);
}

#define new_sh_RL_BUFSIZE 1024
/**
   Read a line of input from stdin.
   return The line from stdin.
 */
char *new_sh_read_line(void)
{
  int bufsize = new_sh_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "new_sh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += new_sh_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "new_sh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define new_sh_TOK_BUFSIZE 64
#define new_sh_TOK_DELIM " \t\r\n\a"

char **new_sh_split_line(char *line)
{
  int bufsize = new_sh_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "new_sh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, new_sh_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += new_sh_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "new_sh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, new_sh_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void new_sh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = new_sh_read_line();
    args = new_sh_split_line(line);
    status = new_sh_execute(args);

    free(line);
    free(args);
  } while (status);
}


