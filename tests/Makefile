CC?=gcc
CFLAGS?=-static -O2 -g -I../include
PAYLOADS:=hello_static payload_chroot_escape payload_cpu_exhaust\
  payload_double_chroot payload_execve_replacement\
  payload_fd_inherited_read payload_mmap_bad_flags payload_mmap_good\
  payload_mmap_prot_exec payload_mmap_self_modify payload_mount_replay\
  payload_ptrace payload_signal_parent payload_socket
.PHONY:all clean setup
all: build/.stamp $(PAYLOADS) blake2b_known seccomp_filter_test
build/.stamp:
	mkdir -p build
	touch $@
$(PAYLOADS):%:%.c
	$(CC) $(CFLAGS) -o build/$@ $<
blake2b_known:blake2b_known.c ../src/util.c
	$(CC) $(CFLAGS) -o build/blake2b_known blake2b_known.c ../src/util.c
seccomp_filter_test:seccomp_filter_test.c ../src/seccomp_bpf.c ../src/log.c
	$(CC) $(CFLAGS) -o build/seccomp_filter_test seccomp_filter_test.c ../src/seccomp_bpf.c ../src/log.c
setup:all
	for d in hello jail empty execve fd_inh mmap_bad mmap_good mmap_protexec mmap_self ptrace socket chroot dbl_chroot mount cpu_exh signal self_hash_match self_hash_mismatch; do mkdir -p "build/roots/$$d/bin"; done
	cp build/hello_static build/roots/hello/bin/
	cp build/hello_static build/roots/jail/bin/
	cp build/hello_static build/roots/empty/bin/
	cp build/payload_execve_replacement build/roots/execve/bin/
	cp build/payload_fd_inherited_read build/roots/fd_inh/bin/
	cp build/payload_mmap_bad_flags build/roots/mmap_bad/bin/
	cp build/payload_mmap_good build/roots/mmap_good/bin/
	cp build/payload_mmap_prot_exec build/roots/mmap_protexec/bin/
	cp build/payload_mmap_self_modify build/roots/mmap_self/bin/
	cp build/payload_ptrace build/roots/ptrace/bin/
	cp build/payload_socket build/roots/socket/bin/
	cp build/payload_chroot_escape build/roots/chroot/bin/
	cp build/payload_double_chroot build/roots/dbl_chroot/bin/
	cp build/payload_mount_replay build/roots/mount/bin/
	cp build/payload_cpu_exhaust build/roots/cpu_exh/bin/
	cp build/payload_signal_parent build/roots/signal/bin/
	cp build/hello_static build/roots/self_hash_match/bin/
	cp build/hello_static build/roots/self_hash_mismatch/bin/
clean:;rm -rf build/
