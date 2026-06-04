#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include "../include/z_jail.h"
#include "../include/seccomp_bpf.h"

static int n_pass, n_fail;
static void pass(const char *name) { n_pass++; printf("  %-40s PASS\n", name); }
static void fail(const char *name, const char *reason) { n_fail++; printf("  %-40s FAIL: %s\n", name, reason); }

static int setup_filter(void) {
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0) return -1;
    if (apply_whitelist() < 0) return -1;
    return 0;
}

/* Blocked syscall test: child installs filter, then calls bad syscall */
static void test_blocked(const char *name, void (*fn)(void)) {
    pid_t pid = fork();
    if (pid < 0) { fail(name, "fork failed"); return; }
    if (pid == 0) {
        if (setup_filter() < 0) _exit(99);
        fn();
        _exit(0);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status) && WTERMSIG(status) == 31)
        pass(name);
    else if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) == 99)
            fail(name, "filter setup failed");
        else
            fail(name, "exited with 0 (should be killed)");
    } else
        fail(name, "unexpected termination");
}

static void test_allowed(const char *name, void (*fn)(void)) {
    pid_t pid = fork();
    if (pid < 0) { fail(name, "fork failed"); return; }
    if (pid == 0) {
        if (setup_filter() < 0) _exit(99);
        fn();
        _exit(0);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        pass(name);
    else if (WIFEXITED(status))
        fail(name, "filter rejected allowed syscall");
    else if (WIFSIGNALED(status))
        fail(name, "was killed unexpectedly");
    else
        fail(name, "unexpected status");
}

static void do_socket(void) { int fd = socket(AF_INET, SOCK_STREAM, 0); if (fd >= 0) close(fd); }
static void do_mmap_bad(void) {
    void *p = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if (p != MAP_FAILED) munmap(p, 4096);
}
static void do_ptrace(void) { ptrace(PTRACE_TRACEME, 0, NULL, NULL); }
static void do_mmap_protexec(void) {
    void *p = mmap(NULL, 4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (p != MAP_FAILED) munmap(p, 4096);
}
static void do_openat(void) { int fd = open("/dev/null", O_RDONLY); if (fd >= 0) close(fd); }
static void do_getpid(void) { syscall(SYS_getpid); }
static void do_mmap_good(void) {
    void *p = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) _exit(1);
    munmap(p, 4096);
}
static void do_write(void) { write(STDOUT_FILENO, "", 0); }

int main(void) {
    printf("=== Seccomp Filter Test ===\n");
    test_blocked("socket", do_socket);
    test_blocked("mmap bad flags (MAP_SHARED)", do_mmap_bad);
    test_blocked("mmap PROT_EXEC", do_mmap_protexec);
    test_blocked("ptrace", do_ptrace);
    test_allowed("openat (via glibc open)", do_openat);
    test_blocked("getpid", do_getpid);
    test_allowed("mmap good flags (MAP_PRIVATE|MAP_ANONYMOUS, RW)", do_mmap_good);
    test_allowed("write", do_write);
    printf("--- %d passed, %d failed ---\n", n_pass, n_fail);
    return n_fail > 0 ? 1 : 0;
}
