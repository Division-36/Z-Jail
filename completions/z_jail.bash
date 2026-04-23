_z_jail() {
    local cur="${COMP_WORDS[COMP_CWORD]}"
    local prev="${COMP_WORDS[COMP_CWORD-1]}"
    case "$prev" in
        --root) COMPREPLY=($(compgen -d -- "$cur")) ;;
        *) COMPREPLY=($(compgen -W "--root= --seccomp-enforce --self-hash --version" -- "$cur")) ;;
    esac
}
complete -F _z_jail z_jail
