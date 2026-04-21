#include "z_jail.h"

seccomp_rule axiom_whitelist[] = {
    {.nr = 0,   .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 1,   .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 257, .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 3,   .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 8,   .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 12,  .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 9,   .arch = AUDIT_ARCH_X86_64, .arg_count = 2,
     .arg_rules = {
        {.nr=9, .arch=AUDIT_ARCH_X86_64, .arg_idx=3, .value=0x22, .mask=0},
        {.nr=9, .arch=AUDIT_ARCH_X86_64, .arg_idx=2, .value=0x0, .mask=0x4},
     }},
    {.nr = 11,  .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 59,  .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 231, .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 13,  .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 14,  .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 318, .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 228, .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
    {.nr = 5,   .arch = AUDIT_ARCH_X86_64, .arg_count = 0},
};
int axiom_whitelist_size = (int)(sizeof(axiom_whitelist)/sizeof(axiom_whitelist[0]));

seccomp_arg_rule axiom_arg_rules[] = {
    {.nr=9, .arch=AUDIT_ARCH_X86_64, .arg_idx=3, .value=0x22, .mask=0},
    {.nr=9, .arch=AUDIT_ARCH_X86_64, .arg_idx=2, .value=0x0, .mask=0x4},
};
int axiom_arg_rules_size = (int)(sizeof(axiom_arg_rules)/sizeof(axiom_arg_rules[0]));
