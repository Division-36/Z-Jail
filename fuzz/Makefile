CC=clang
CFLAGS=-fsanitize=fuzzer -g -I../include
fuzz_bpf: fuzz_bpf.c ../src/seccomp_bpf.c ../src/log.c
	$(CC) $(CFLAGS) -o $@ fuzz_bpf.c ../src/seccomp_bpf.c ../src/log.c
clean:;rm -f fuzz_bpf
