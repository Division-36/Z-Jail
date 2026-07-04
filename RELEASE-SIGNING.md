# Release Signing

v1 release artifacts are signed with [minisign](https://jedisct1.github.io/minisign/).

## Public Key

```
RWTC0j7RhnUtpjGbhcm/dKRiz8k8IW4z069gZZzhhjbAokPenWYgVliH
```

## Verification

```sh
minisign -Vm z-jail-v1.tar.gz -P "RWTC0j7RhnUtpjGbhcm/dKRiz8k8IW4z069gZZzhhjbAokPenWYgVliH"
```

## Signing Process

1. Build `make dist`
2. Sign: `minisign -Sm _dist/z-jail-v1.tar.gz`
3. Upload: `_dist/z-jail-v1.tar.gz`, `_dist/z-jail-v1.tar.gz.minisig`
