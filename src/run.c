#include "utils.h"

#include "run.h"
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int run(struct command cmd, char **env, char** aliases) {
  switch (cmd.builtin) {
  case NONE:
    run_cmd(cmd, env, aliases);
    break;
  case CD:
    run_cd(cmd);
    break;
  case EXIT:
    return 1;
  case ECHO:
    echo(cmd);
    break;
  case ALIAS:
    run_alias(cmd, aliases);
    break;
  case EMPTY:
    break;
  }
  return 0;
}

void run_cmd(struct command cmd, char **env, char** aliases) {

  int hash_value = MOD(hash(cmd.arg_ptrs[0], strlen(cmd.arg_ptrs[0])), 128);
  
  if (aliases[hash_value] != 0) {
    //printf("%s\n",aliases[hash_value]);
    cmd.arg_ptrs[0] = aliases[hash_value];
  }
  
  pid_t pid; 
  if (!cmd.bg) {
    int result =
        posix_spawnp(&pid, cmd.arg_ptrs[0], NULL, NULL, cmd.arg_ptrs, env);
    switch (result) {
        case 0: waitpid(pid, &result, 0);
                break;
        case 2: printf("-kash: %s: command not found\n", cmd.arg_ptrs[0]);
                break;
        default: printf("-kash: %s\n", strerror(result));
    }
  } else {
    posix_spawnp(&pid, cmd.arg_ptrs[0], NULL, NULL, cmd.arg_ptrs, env);
  }
}

void run_cd(struct command cmd) {
  char *dir;
  if (cmd.arg_ptrs[1] == 0)
      dir = getenv("HOME");
  else if (cmd.arg_ptrs[1][0] == '-' && cmd.arg_ptrs[1][1] == 0)
      dir = getenv("OLDPWD");
  else
      dir = cmd.arg_ptrs[1];
      
  if (chdir(dir)) { // this is true if cd returns an error
    printf("-kash: cd: %s: No such file or directory\n", dir);
  }
}


void echo(struct command cmd) {
    int i = 1;
    while (cmd.arg_ptrs[i] != 0 && i < MAX_ARGS - 1) {
        if (cmd.arg_ptrs[i + 1])
            printf("%s ", cmd.arg_ptrs[i]);
        else 
            printf("%s\n", cmd.arg_ptrs[i]);
        i += 1;
    }
}

void run_alias(struct command cmd, char** aliases) {
  int hash_value = MOD(hash(cmd.arg_ptrs[1], strlen(cmd.arg_ptrs[1])), 128);
  aliases[hash_value] = malloc(strlen(cmd.arg_ptrs[2])*sizeof(char));
  memcpy(aliases[hash_value], cmd.arg_ptrs[2], strlen(cmd.arg_ptrs[2])); 
  //printf("%s\n%s\n", cmd.arg_ptrs[1], aliases[MOD(hash(cmd.arg_ptrs[1], strlen(cmd.arg_ptrs[1])), 128)]);
}