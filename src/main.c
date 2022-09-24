#include "utils.h"

#include "keys.h"
#include "main.h"
#include "parse.h"
#include "prompt.h"
#include "run.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

void intHandler(int dummy) {}

int main(int argc, char **argv, char **envp) {
  signal(SIGINT, intHandler);
  int opt;
  char inp[MAX_INPUT] = {0};
  enum session_t tsession = INTERACTIVE;
  while ((opt = getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
    case 'c':
      memcpy(inp, optarg, MAX_INPUT);
      inp[MAX_INPUT - 1] = 0;
      tsession = NONINTERACTIVE;
      break;
    }
  }

  if (!isatty(STDIN_FILENO)) {
    shell_loop(envp, NONINTERACTIVE, STDIN_FILENO, NULL);
  } else if (optind >= argc) {
    shell_loop(envp, tsession, 0, inp);
  } else {
    int i = optind;
    while (i < argc) {
      int fp = open(argv[i], O_RDONLY);
      if (fp == 0) {
        printf("-kash: unable to open file '%s': %s\n", argv[i],
               strerror(errno));
        return 1;
      } else {
        shell_loop(envp, NONINTERACTIVE, fp, NULL);
        close(fp);
      }
      i++;
    }
    return 0;
  }
}

int shell_loop(char **env, int sess, int input_fd, char *input_str) {
  struct command cmd;
  char history[MAX_HISTORY][MAX_INPUT];
  for (int i = 0; i <= MAX_HISTORY; i++) {
    history[i][0] = '\0';
  }
  char input[MAX_INPUT] = "";
  char prompt[MAX_PROMPT] = "";

  int history_idx = 0;
  while (1) {
    if (sess == INTERACTIVE) {
      print_prompt(prompt);
      handle_keys(input, history, history_idx);
    } else {
      if (input_str != 0) {
        memcpy(input, input_str, MAX_INPUT);
      } else {
        read(input_fd, input, MAX_INPUT);
      }
      input[MAX_INPUT - 1] = 0;
    }
    strcpy(history[history_idx], input);
    history_idx = (history_idx + 1) % MAX_HISTORY;
    char *line_ret;
    for (char *line = strtok_r(input, "\n;", &line_ret); line;
         line = strtok_r(NULL, "\n;", &line_ret)) {
      parse_input(line, &cmd);
      if (run(cmd, env))
        return EXIT_SUCCESS;
    }
    if (sess == NONINTERACTIVE)
      break;
  }
  return EXIT_SUCCESS;
}
