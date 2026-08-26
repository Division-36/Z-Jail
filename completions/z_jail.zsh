#compdef z_jail
_arguments \
  '--root=[sandbox root filesystem]:directory:_files -/' \
  '--seccomp-enforce[enable seccomp-BPF]' \
  '--self-hash[verify binary integrity]' \
  '--quiet[suppress audit output]' \
  '--no-audit[disable audit file creation]' \
  '--verbose[enable debug logging]' \
  '--version[show build ID]' \
  '--help[show usage]'
