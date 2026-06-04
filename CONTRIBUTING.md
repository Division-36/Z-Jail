# Contributing

Bug reports, feature requests, and pull requests are welcome.

## Before You Submit

- **No external dependencies** — patches that pull in libseccomp, libcap, or similar will be rejected. We roll our own BPF, capabilities, and hashing.
- **Keep it minimal** — Z-Jail is ~900 LoC of core logic. New features should justify their size.
- **Test it** — add or update test scenarios in `tests/`. Run `make -C tests setup && bash tests/run_tests.sh` (needs root).
- **No new sycalls in the whitelist without ADR** — seccomp changes require an Architecture Decision Record in `docs/adr/`.

## Pull Request Process

1. Fork the repo, create a feature branch
2. Make your change, add tests
3. Run `make` and verify the test suite passes
4. Submit the PR with a clear description of what and why

## Reporting Issues

Open an issue at https://github.com/Division-36/Z-Jail/issues or email **zs.01117875692@gmail.com**.

## Code Style

- C11 / gnu11 (BPF macros need `gnu11`)
- Tabs for indentation, no trailing whitespace
- No comments unless the code genuinely can't speak for itself
- Function-level `AXIOM_STATIC_ASSERT` for critical invariants
