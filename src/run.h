int run(struct command cmd, char **env, char** aliases);
void run_cmd(struct command cmd, char **env, char** aliases);
void run_cd(struct command cmd);
void echo(struct command cmd);
void run_alias(struct command cmd, char** aliases);
