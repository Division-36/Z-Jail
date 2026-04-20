#pragma once

typedef struct {
    int      nr;
    int      arch;
    uint32_t arg_idx;
    uint32_t value;
    uint32_t mask;
} seccomp_arg_rule;

typedef struct {
    int nr;
    int arch;
    int arg_count;
    seccomp_arg_rule arg_rules[6];
} seccomp_rule;

extern seccomp_rule axiom_whitelist[];
extern int axiom_whitelist_size;
extern seccomp_arg_rule axiom_arg_rules[];
extern int axiom_arg_rules_size;

int apply_whitelist(void);
