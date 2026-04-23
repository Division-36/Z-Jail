# Sandbox
Layer ordering: rlimits -> fd scrub -> dumpable=0 -> pivot_root ->
NO_NEW_PRIVS -> drop_caps -> seccomp -> signal parent -> execve
pivot_root recipe: bind mount -> pivot_root -> chdir("/") -> MNT_DETACH -> rmdir
