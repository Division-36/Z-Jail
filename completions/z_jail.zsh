#compdef z_jail
_arguments \
  '--root=[sandbox root filesystem]:directory:_files -/' \
  '--seccomp-enforce[enable seccomp-BPF]' \
  '--self-hash[verify binary integrity]' \
  '--version[show build ID]'
