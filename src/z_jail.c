#include "z_jail.h"
#include <sys/wait.h>

extern char **environ;

static int parse_args(int argc, char **argv,
    sandbox_config *cfg, char **self_hash_hex)
{
    int i, cmd_start = -1;

    cfg->root_path = ".";
    cfg->seccomp_enforce = 0;
    cfg->timeout_sec = 30;
    cfg->as_limit = 256ULL * 1024 * 1024;
    *self_hash_hex = NULL;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            cmd_start = i + 1;
            break;
        }
        if (strncmp(argv[i], "--root=", 7) == 0)
            cfg->root_path = argv[i] + 7;
        else if (strcmp(argv[i], "--seccomp-enforce") == 0)
            cfg->seccomp_enforce = 1;
        else if (strncmp(argv[i], "--self-hash=", 12) == 0)
            *self_hash_hex = argv[i] + 12;
        else if (strcmp(argv[i], "--verbose") == 0)
            axiom_log_level = LOG_DEBUG;
        else if (argv[i][0] == '-') {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            return -1;
        } else {
            cmd_start = i;
            break;
        }
    }

    if (cmd_start < 0 || cmd_start >= argc) {
        fprintf(stderr, "usage: z_jail [opts] <cmd> [args...]\n");
        return -1;
    }

    cfg->exec_path = argv[cmd_start];
    cfg->exec_argv = argv + cmd_start;
    cfg->exec_envp = environ;
    return 0;
}

int main(int argc, char **argv)
{
    sandbox_config cfg;
    char *self_hash_hex = NULL;

    if (parse_args(argc, argv, &cfg, &self_hash_hex) < 0)
        return 1;

    axiom_log(LOG_INFO, "target: %s\n", cfg.exec_path);
    return 0;
}
