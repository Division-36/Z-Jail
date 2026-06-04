#pragma once
#define _GNU_SOURCE
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sched.h>
#include <signal.h>
#include <time.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/capability.h>
#include "truthimatics.h"
#include "sandbox.h"
#include "util.h"
#include "audit.h"
#include "seccomp_bpf.h"
#define ZJAIL_BUILD_ID   "Z-Jail/v1+dev"
#define ZJAIL_VERSION    "v1"
#define LOG_NONE  0
#define LOG_ERROR 1
#define LOG_WARN  2
#define LOG_INFO  3
#define LOG_DEBUG 4
extern int axiom_log_level;
void axiom_log(int level, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
#define AXIOM_CHILD_OK         0
#define AXIOM_CHILD_ERR_SETUP  101
#define AXIOM_CHILD_ERR_SECCOMP 102
#define AXIOM_CHILD_ERR_EXEC   103
#define AXIOM_CHILD_ERR_PERM   104
#define AXIOM_CHILD_ERR_CAP    105
#define AXIOM_NORETURN __attribute__((noreturn))
#define AXIOM_UNUSED   __attribute__((unused))
#define AXIOM_IGNORE_RESULT(expr) \
	do { volatile __typeof__(expr) _axiom_ign_ = (expr); (void)_axiom_ign_; } while (0)
#define AXIOM_PRINTF(f,a) __attribute__((format(printf,f,a)))
#define AXIOM_STATIC_ASSERT(cond, msg) \
	typedef char AXIOM_UNUSED _axiom_sa_##msg[(cond) ? 1 : -1]
int axiom_evidence_add_fn(axiom_evidence_chain *chain,
	const char *id, axiom_weight weight,
	axiom_evidence_fn fn, void *ctx, const char *target);
int axiom_child(void *arg);
void axiom_fatal(const char *msg) AXIOM_NORETURN;
