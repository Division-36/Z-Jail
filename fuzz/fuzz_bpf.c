// Fuzz target for seccomp BPF generation
#include "../include/seccomp_bpf.h"
#include <stdint.h>
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    (void)data; (void)size;
    struct sock_fprog prog;
    axiom_seccomp_bpf_build(&prog);
    return 0;
}
