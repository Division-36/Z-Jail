# Benchmarks

Reports stored in `_benchmarks/reports/`.

## Metrics
- **Latency**: time from clone() to execve() exit
- **Memory**: RSS delta between parent before clone and after wait
- **Fanout**: overhead per additional concurrent sandbox

## Running
```sh
make -C _benchmarks run
```

## Current numbers (WSL2, GCC 15.2.0, -O2 -g)

| Metric | Value |
|--------|-------|
| Mean latency | ~8 ms |
| Peak RSS | ~4 MiB |
| Binary size | ~130 KiB |
