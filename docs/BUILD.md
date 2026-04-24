# Build
## Requirements
GCC 11+, Linux 5.4+, no external deps.
## Commands
```
make          # z_jail (~130 KiB PIE)
make clean
make build_id
```
## Test
```
make -C tests setup
bash tests/run_tests.sh
```
