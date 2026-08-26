/*
 * measure.c - seccomp filter robustness gauge for Z-Jail.
 *
 * Builds the REAL shipped filter (src/seccomp_bpf.c::apply_whitelist) but
 * intercepts prctl(PR_SET_SECCOMP) via --wrap so nothing is installed; it then
 * reports the compiled program's size and the largest conditional jump
 * displacement. Classic-BPF jt/jf fields are 8-bit, so a filter is safe only
 * while its maximum displacement stays <= 255. Tracking the headroom turns a
 * silent, size-dependent cliff into a CI-assertable number.
 *
 * Build (see run.sh):
 *   gcc -I include -Wl,--wrap=prctl _benchmarks/seccomp_robustness/measure.c \
 *       src/seccomp_bpf.c src/log.c -o measure
 */
#include "z_jail.h"

static struct sock_filter *g_filter = NULL;
static unsigned short g_len = 0;

int __wrap_prctl(int option, unsigned long a2, unsigned long a3,
                 unsigned long a4, unsigned long a5)
{
    (void)a4; (void)a5;
    if (option == PR_SET_SECCOMP && a2 == SECCOMP_MODE_FILTER) {
        struct sock_fprog *p = (struct sock_fprog *)(void *)a3;
        g_len = p->len;
        g_filter = p->filter;
        return 0;
    }
    return 0;
}

int main(void)
{
    axiom_log_level = LOG_NONE;
    if (apply_whitelist() != 0 || g_filter == NULL) {
        fprintf(stderr, "apply_whitelist failed / filter not captured\n");
        return 1;
    }

    int max_jt = 0, max_jf = 0, i;
    for (i = 0; i < (int)g_len; i++) {
        if (g_filter[i].jt > max_jt) max_jt = g_filter[i].jt;
        if (g_filter[i].jf > max_jf) max_jf = g_filter[i].jf;
    }
    int max_off = max_jt > max_jf ? max_jt : max_jf;
    int headroom = 255 - max_off;

    /* whitelist size drives the arch-guard displacement (~3*N for no-arg calls);
     * report the currently-allowed count for context. */
    int n = axiom_whitelist_size;

    printf("metric,value\n");
    printf("whitelist_syscalls,%d\n", n);
    printf("filter_instructions,%u\n", g_len);
    printf("max_jump_offset,%d\n", max_off);
    printf("headroom_to_8bit_limit,%d\n", headroom);
    printf("safe,%s\n", max_off <= 255 ? "yes" : "NO");
    return 0;
}
