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

#define ARCH_OFFSET   4
#define NR_OFFSET     0
#define ARG_OFFSET(i) (16 + (i) * 8)

static int count_instructions(void)
{
    int count = 2, i, j;
    count += axiom_whitelist_size * 2;
    count += 1;
    for (i = 0; i < axiom_whitelist_size; i++) {
        if (axiom_whitelist[i].arg_count > 0) {
            for (j = 0; j < axiom_whitelist[i].arg_count; j++) {
                if (axiom_whitelist[i].arg_rules[j].mask != 0) count += 3;
                else count += 2;
            }
            count += 1;
        } else { count += 1; }
    }
    return count;
}

int apply_whitelist(void)
{
    struct sock_filter *filter;
    struct sock_fprog prog;
    int total, pos, i, j, *check_pos, kill_pos;

    total = count_instructions();
    filter = calloc((size_t)total, sizeof(*filter));
    if (!filter) { axiom_log(LOG_ERROR,"seccomp: calloc\n"); return -1; }
    check_pos = calloc((size_t)axiom_whitelist_size, sizeof(int));
    if (!check_pos) { free(filter); return -1; }

    pos = 0;
    filter[pos++] = BPF_STMT(BPF_LD|BPF_W|BPF_ABS, ARCH_OFFSET);
    filter[pos++] = BPF_JUMP(BPF_JMP|BPF_JEQ, AUDIT_ARCH_X86_64, 1, 0);

    for (i = 0; i < axiom_whitelist_size; i++) {
        filter[pos++] = BPF_STMT(BPF_LD|BPF_W|BPF_ABS, NR_OFFSET);
        filter[pos++] = BPF_JUMP(BPF_JMP|BPF_JEQ, (uint32_t)axiom_whitelist[i].nr, 0, 0);
    }

    kill_pos = pos;
    filter[pos++] = BPF_STMT(BPF_RET, SECCOMP_RET_KILL);

    for (i = 0; i < axiom_whitelist_size; i++) {
        check_pos[i] = pos;
        if (axiom_whitelist[i].arg_count > 0) {
            for (j = 0; j < axiom_whitelist[i].arg_count; j++) {
                seccomp_arg_rule *r = &axiom_whitelist[i].arg_rules[j];
                int off = ARG_OFFSET(r->arg_idx);
                if (r->mask != 0) {
                    filter[pos++] = BPF_STMT(BPF_LD|BPF_W|BPF_ABS, off);
                    filter[pos++] = BPF_STMT(BPF_ALU|BPF_AND, r->mask);
                    filter[pos++] = BPF_JUMP(BPF_JMP|BPF_JEQ, r->value, 0, 1);
                } else {
                    filter[pos++] = BPF_STMT(BPF_LD|BPF_W|BPF_ABS, off);
                    filter[pos++] = BPF_JUMP(BPF_JMP|BPF_JEQ, r->value, 0, 1);
                }
            }
            filter[pos++] = BPF_STMT(BPF_RET, SECCOMP_RET_ALLOW);
        } else {
            filter[pos++] = BPF_STMT(BPF_RET, SECCOMP_RET_ALLOW);
        }
    }

    for (i = 0; i < axiom_whitelist_size; i++) {
        int nr_idx = 2 + i*2 + 1;
        int jt = check_pos[i] - (nr_idx + 1);
        int jf = kill_pos - (nr_idx + 1);
        filter[nr_idx].jt = (uint8_t)(jt > 255 ? 255 : jt);
        filter[nr_idx].jf = (uint8_t)(jf > 255 ? 255 : jf);
    }
    filter[1].jt = (uint8_t)((2-(1+1)) > 255 ? 255 : (2-(1+1)));
    filter[1].jf = (uint8_t)((kill_pos-(1+1)) > 255 ? 255 : (kill_pos-(1+1)));

    prog.len = (unsigned short)total;
    prog.filter = filter;

    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog, 0, 0) < 0) {
        axiom_log(LOG_ERROR,"seccomp: PR_SET_SECCOMP: %s\n",strerror(errno));
        free(filter); free(check_pos); return -1;
    }
    free(check_pos);
    return 0;
}

AXIOM_STATIC_ASSERT(sizeof(struct sock_filter) == 8, sock_filter_size);

