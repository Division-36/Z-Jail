# Release Signing

v1 release artifacts are signed with [minisign](https://jedisct1.github.io/minisign/).

## Public Key

```
RWR—————————————————————————————————————
```

*Key fingerprint not yet established. Will be published here and on key servers.*

## Verification

```sh
minisign -Vm z-jail-v1.tar.gz -P "RWR—————————————————————————————————————"
```

## Signing Process

1. Build `make dist`
2. Sign: `minisign -Sm _dist/z-jail-v1.tar.gz`
3. Upload: `_dist/z-jail-v1.tar.gz`, `_dist/z-jail-v1.tar.gz.minisig`
