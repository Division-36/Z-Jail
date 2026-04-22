#pragma once

typedef struct {
    int         report_fd;
    int         seccomp_enforce;
    char       *root_path;
    char       *exec_path;
    char      **exec_argv;
    char      **exec_envp;
    int         timeout_sec;
    unsigned long long as_limit;
} sandbox_config;

int child_run(void *arg);
