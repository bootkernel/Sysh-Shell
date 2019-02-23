#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int procesh_cd(char **args);
int procesh_help(char **args);
int procesh_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &procesh_cd,
  &procesh_help,
  &procesh_exit
};

int procesh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int procesh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "procesh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("procesh");
    }
  }
  return 1;
}

int procesh_help(char **args)
{
  int i;
  printf("Procesh Shell - created by Subhajeet Mukherjee\n");
  printf("Here are the currenty available commands:\n");
  printf("cd - Changes between directories\nhelp - see available commands and their functions\nexit - exits the shell\n");
  return 1;
}

int procesh_exit(char **args) {
  return 0;
}

int procesh_launch(char **args) {
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("procesh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("procesh");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int procesh_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < procesh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return procesh_launch(args);
}

#define procesh_LIMIT 1024

char *procesh_read_line(void)
{
  int Size_Buffer = procesh_LIMIT;
  int position = 0;
  char *buffer = malloc(sizeof(char) * Size_Buffer);
  int c;

  if (!buffer) {
    fprintf(stderr, "procesh: allocation issue\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    if (position >= Size_Buffer) {
      Size_Buffer += procesh_LIMIT;
      buffer = realloc(buffer, Size_Buffer);
      if (!buffer) {
        fprintf(stderr, "procesh: allocation issue\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define procesh_TOK_BUFSIZE 64
#define procesh_TOK_DELIM " \t\r\n\a"

char **procesh_separateLines(char *line) {

  int Size_Buffer = procesh_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(Size_Buffer * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "procesh: allocation issue\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, procesh_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= Size_Buffer) {
      Size_Buffer += procesh_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, Size_Buffer * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "procesh: allocation issue\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, procesh_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
  }

void procesh_runtime(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("procesh-0.1$ ");
    line = procesh_read_line();
    args = procesh_separateLines(line);
    status = procesh_execute(args);

    free(line);
    free(args);
  } while (status);
}
int main(int argc, char **arg) {
  procesh_runtime();

  return EXIT_SUCCESS;
}

