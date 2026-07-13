#!/usr/bin/env bash
#
# full_native_benchmark.sh
# ------------------------------------------------------------------------------
# One-shot, self-contained benchmark harness for the Z-Jail / TAC paper.
#
# It will, on a NATIVE Linux machine:
#   1. detect the distro and install every dependency (build tools, bubblewrap,
#      nsjail -- building nsjail from source if no package exists);
#   2. build z_jail from the repository (cloning it if not already present);
#   3. build an identical freestanding, statically linked workload whose entire
#      body is exit_group(0) (no libc), and a wait4-based measurement harness;
#   4. run 50 timed samples (after 5 warm-ups) per sandbox using ONE uniform
#      methodology: wall-clock latency (CLOCK_MONOTONIC) + peak RSS (ru_maxrss);
#   5. run the project's own 18-scenario functional test suite;
#   6. collect full system/provenance info and emit a single Markdown REPORT.md
#      plus the raw CSVs so the numbers are fully reproducible.
#
# USAGE:
#   bash full_native_benchmark.sh                 # root or sudo: installs deps, clones repo
#   REPO_DIR=/path/to/Z-Jail bash full_native_benchmark.sh   # use a local checkout
#
# CONTAINER / NO-SUDO: the script runs fine with no root and no sudo. In that
# case it does NOT try to install packages -- it assumes a build toolchain
# (git, gcc, make) is already present, and simply skips any sandbox it cannot
# run (e.g. z_jail/nsjail if the container forbids namespace creation).
# bubblewrap works when user namespaces are enabled.
#
# Tunables (env vars):
#   ITERS=50 WARMUP=5 REPO_URL=... REPO_DIR=... OUTDIR=...
# ------------------------------------------------------------------------------

set -uo pipefail

# ------------------------------- configuration --------------------------------
ITERS="${ITERS:-50}"
WARMUP="${WARMUP:-5}"
REPO_URL="${REPO_URL:-https://github.com/Division-36/Z-Jail.git}"
REPO_DIR="${REPO_DIR:-}"
TS="$(date +%Y%m%d_%H%M%S)"
OUTDIR="${OUTDIR:-$PWD/bench_results_$TS}"
BUILD="$OUTDIR/build"
CSVDIR="$OUTDIR/csv"
REPORT="$OUTDIR/REPORT.md"

# ------------------------------- pretty logging -------------------------------
if [ -t 1 ]; then
    C_R="\033[31m"; C_G="\033[32m"; C_Y="\033[33m"; C_B="\033[34m"; C_0="\033[0m"
else
    C_R=""; C_G=""; C_Y=""; C_B=""; C_0=""
fi
log()  { printf "${C_B}==>${C_0} %s\n" "$*"; }
ok()   { printf "${C_G}  ok:${C_0} %s\n" "$*"; }
warn() { printf "${C_Y}  warn:${C_0} %s\n" "$*"; }
err()  { printf "${C_R}  err:${C_0} %s\n" "$*" >&2; }

mkdir -p "$BUILD" "$CSVDIR"

# --------------------------- privilege / sudo setup ---------------------------
# Works with root, with sudo, or with neither (e.g. inside a container).
if [ "$(id -u)" -eq 0 ]; then
    SUDO=""; IS_ROOT=1; CAN_INSTALL=1
else
    IS_ROOT=0
    if command -v sudo >/dev/null 2>&1; then SUDO="sudo"; CAN_INSTALL=1
    else SUDO=""; CAN_INSTALL=0; fi
fi
if [ "$IS_ROOT" -eq 0 ]; then
    warn "not running as root."
    [ "$CAN_INSTALL" -eq 0 ] && \
        warn "no sudo either: dependency installation is SKIPPED (assuming build tools are preinstalled)."
    warn "z_jail/nsjail need namespace privileges; in an unprivileged container they may be skipped (that is fine, other tools still run)."
fi

# ----------------------------- distro detection -------------------------------
PKG=""; DISTRO="unknown"
if [ -r /etc/os-release ]; then . /etc/os-release; DISTRO="${ID:-unknown}"; fi
if   command -v apt-get >/dev/null 2>&1; then PKG="apt"
elif command -v dnf     >/dev/null 2>&1; then PKG="dnf"
elif command -v yum     >/dev/null 2>&1; then PKG="yum"
elif command -v pacman  >/dev/null 2>&1; then PKG="pacman"
elif command -v zypper  >/dev/null 2>&1; then PKG="zypper"
fi
log "distro=$DISTRO  package-manager=${PKG:-none}"

# Wait until the dpkg/apt locks are free. Fresh cloud images run
# unattended-upgrades on first boot and hold these locks for minutes, which is
# exactly what made package installs fail silently before. Returns 0 when free.
wait_apt_free() {
    local tries=72   # up to ~6 min
    while [ "$tries" -gt 0 ]; do
        if fuser /var/lib/dpkg/lock-frontend /var/lib/dpkg/lock /var/lib/apt/lists/lock \
                >/dev/null 2>&1; then
            sleep 5; tries=$((tries-1)); continue
        fi
        if pgrep -x apt-get >/dev/null 2>&1 || pgrep -x dpkg >/dev/null 2>&1 \
                || pgrep -x unattended-upgrade >/dev/null 2>&1; then
            sleep 5; tries=$((tries-1)); continue
        fi
        return 0
    done
    return 1
}

pkg_install() {
    # pkg_install <generic names...>  -- best-effort, never fatal
    [ "${CAN_INSTALL:-0}" -eq 0 ] && { warn "cannot install (no root/sudo); assuming preinstalled: $*"; return 0; }
    [ -z "$PKG" ] && { warn "no package manager; skipping install of: $*"; return 0; }
    case "$PKG" in
        apt)
            # Stop first-boot unattended-upgrades so it stops hogging the lock.
            $SUDO systemctl stop unattended-upgrades 2>/dev/null
            $SUDO systemctl disable unattended-upgrades 2>/dev/null
            $SUDO pkill -9 -f unattended-upgrade 2>/dev/null
            wait_apt_free || warn "apt lock still busy after waiting; install may fail"
            export DEBIAN_FRONTEND=noninteractive
            local logf
            logf="$(mktemp)"
            for attempt in 1 2 3; do
                $SUDO apt-get update -y >"$logf" 2>&1
                if $SUDO apt-get install -y "$@" >>"$logf" 2>&1; then
                    rm -f "$logf"; return 0
                fi
                sleep 3
            done
            warn "apt: some of [$*] failed (last apt output follows)"
            tail -20 "$logf" >&2
            rm -f "$logf"
            ;;
        dnf) $SUDO dnf install -y "$@"    >/dev/null 2>&1 || warn "dnf: some of [$*] failed" ;;
        yum) $SUDO yum install -y "$@"    >/dev/null 2>&1 || warn "yum: some of [$*] failed" ;;
        pacman) $SUDO pacman -Sy --noconfirm "$@" >/dev/null 2>&1 || warn "pacman: some of [$*] failed" ;;
        zypper) $SUDO zypper -n install "$@" >/dev/null 2>&1 || warn "zypper: some of [$*] failed" ;;
    esac
}

# ------------------------------ dependencies ----------------------------------
log "installing base build dependencies"
case "$PKG" in
    apt)    pkg_install git gcc g++ make binutils coreutils gawk file bubblewrap \
                        pkg-config ;;
    dnf|yum) pkg_install git gcc gcc-c++ make binutils coreutils gawk file \
                        bubblewrap pkgconf-pkg-config ;;
    pacman) pkg_install git gcc make binutils coreutils gawk file bubblewrap \
                        pkgconf ;;
    zypper) pkg_install git gcc gcc-c++ make binutils coreutils gawk file \
                        bubblewrap pkg-config ;;
    *)      warn "unknown package manager; ensure git/gcc/make/bubblewrap exist" ;;
esac

# Fail early and clearly if the build toolchain is genuinely missing.
MISSING=""
for t in gcc make; do command -v "$t" >/dev/null 2>&1 || MISSING="$MISSING $t"; done
if [ -n "$MISSING" ]; then
    err "missing build tools:$MISSING (and they could not be installed)."
    err "install them manually, then re-run. e.g. apt-get install -y gcc make git"
    exit 1
fi
command -v git >/dev/null 2>&1 || warn "git missing; you must pass REPO_DIR=/path/to/checkout"

# nsjail: try a package first, otherwise build from source.
install_nsjail() {
    if command -v nsjail >/dev/null 2>&1; then ok "nsjail already present"; return 0; fi
    log "attempting nsjail package install"
    case "$PKG" in
        apt)    pkg_install nsjail ;;
        dnf|yum) pkg_install nsjail ;;
        pacman) pkg_install nsjail ;;
        *) : ;;
    esac
    command -v nsjail >/dev/null 2>&1 && { ok "nsjail installed from package"; return 0; }

    log "building nsjail from source"
    case "$PKG" in
        apt) pkg_install autoconf bison flex libprotobuf-dev libnl-route-3-dev \
                         libtool protobuf-compiler ;;
        dnf|yum) pkg_install autoconf bison flex protobuf-devel libnl3-devel \
                         libtool protobuf-compiler ;;
        pacman) pkg_install autoconf bison flex protobuf libnl libtool ;;
        zypper) pkg_install autoconf bison flex protobuf-devel libnl3-devel \
                         libtool ;;
    esac
    ( cd "$BUILD" && \
      git clone --depth=1 https://github.com/google/nsjail.git nsjail-src >/dev/null 2>&1 && \
      cd nsjail-src && timeout -k 5 900 make -j"$(nproc)" >/dev/null 2>&1 && \
      [ -x ./nsjail ] && cp ./nsjail "$BUILD/nsjail" )
    if [ -x "$BUILD/nsjail" ]; then
        ok "nsjail built from source -> $BUILD/nsjail"
        export PATH="$BUILD:$PATH"
    else
        warn "nsjail build failed; it will be skipped in the benchmark"
    fi
}
install_nsjail

# --------------------------- obtain the repository ----------------------------
if [ -z "$REPO_DIR" ]; then
    REPO_DIR="$BUILD/Z-Jail"
    if [ ! -d "$REPO_DIR/.git" ]; then
        log "cloning $REPO_URL"
        git clone --depth=1 "$REPO_URL" "$REPO_DIR" >/dev/null 2>&1 \
            || { err "git clone failed; set REPO_DIR=/path/to/checkout"; exit 1; }
    fi
fi
[ -f "$REPO_DIR/Makefile" ] || { err "no Makefile in REPO_DIR=$REPO_DIR"; exit 1; }
log "using repository: $REPO_DIR"

# ------------------------------- build z_jail ---------------------------------
log "building z_jail"
( cd "$REPO_DIR" && make >/dev/null 2>&1 ) || warn "make (default) reported issues"
ZJAIL=""
for cand in "$REPO_DIR/z_jail" "$REPO_DIR/build/z_jail" "$REPO_DIR/src/z_jail"; do
    [ -x "$cand" ] && { ZJAIL="$cand"; break; }
done
[ -n "$ZJAIL" ] && ok "z_jail -> $ZJAIL" || err "z_jail binary not found after build"

# ---------------------- freestanding workload + harness -----------------------
log "compiling freestanding workload (exit_group(0), no libc)"
cat > "$BUILD/workload.c" <<'EOF'
/* freestanding: the whole program is exit_group(0). No libc, no _start deps. */
void _start(void) {
    __asm__ volatile(
        "mov $231, %%rax\n"   /* SYS_exit_group */
        "xor %%rdi, %%rdi\n"  /* status = 0 */
        "syscall\n"
        ::: "rax", "rdi");
}
EOF
gcc -static -nostdlib -no-pie -O2 -s -o "$BUILD/workload" "$BUILD/workload.c" \
    2>/dev/null || gcc -static -nostdlib -O2 -s -o "$BUILD/workload" "$BUILD/workload.c"
[ -x "$BUILD/workload" ] && ok "workload built ($(stat -c%s "$BUILD/workload") bytes)" \
    || { err "workload failed to build"; exit 1; }

log "compiling wait4-based measurement harness"
cat > "$BUILD/harness.c" <<'EOF'
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "usage: %s <warmup> <iters> <cmd> [args...]\n", argv[0]);
        return 2;
    }
    int warmup = atoi(argv[1]);
    int iters  = atoi(argv[2]);
    char **cmd = &argv[3];
    int devnull = open("/dev/null", O_WRONLY);

    printf("iter,wall_ns,rss_kib,exit_code\n");
    for (int i = 0; i < warmup + iters; i++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        pid_t pid = fork();
        if (pid == 0) {
            if (devnull >= 0) { dup2(devnull, 1); dup2(devnull, 2); }
            execvp(cmd[0], cmd);
            _exit(127);
        }
        int status; struct rusage ru;
        wait4(pid, &status, 0, &ru);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        long long wall = (long long)(t1.tv_sec - t0.tv_sec) * 1000000000LL
                       + (t1.tv_nsec - t0.tv_nsec);
        int ec = WIFEXITED(status) ? WEXITSTATUS(status) : -(WTERMSIG(status));
        if (i >= warmup)
            printf("%d,%lld,%ld,%d\n", i - warmup, wall, ru.ru_maxrss, ec);
    }
    return 0;
}
EOF
gcc -O2 -o "$BUILD/harness" "$BUILD/harness.c" || { err "harness build failed"; exit 1; }
ok "harness built"

# ------------------------------ sandbox roots ---------------------------------
# Put the sandbox root in /tmp: bubblewrap (when run as root) cannot bind-mount
# a source path that lives under /home, so a world-readable /tmp root is used.
ROOT="$(mktemp -d /tmp/zjroot.XXXXXX)"
chmod 0755 "$ROOT"
mkdir -p "$ROOT/bin"
cp "$BUILD/workload" "$ROOT/bin/workload"
chmod 0755 "$ROOT/bin/workload"

# Exact commands (recorded verbatim in the report).
ZJAIL_CMD=( "$ZJAIL" "--root=$ROOT" "--seccomp-enforce" "--" "bin/workload" )
BWRAP_CMD=( bwrap --unshare-all --die-with-parent --ro-bind "$ROOT" / -- /bin/workload )
NSJAIL_BIN="$(command -v nsjail || echo "$BUILD/nsjail")"
NSJAIL_CMD=( "$NSJAIL_BIN" -Mo -q --chroot "$ROOT" --disable_proc --disable_clone_newuser -- /bin/workload )

# ------------------------------ sanity checks ---------------------------------
declare -A TOOL_OK
sanity() {
    # sanity <name> <cmd...>  -- bounded by timeout so a hanging tool can't stall us
    local name="$1"; shift
    timeout -k 5 30 "$@" >/dev/null 2>&1
    local rc=$?
    if [ $rc -eq 0 ]; then TOOL_OK[$name]=1; ok "$name runs the workload (exit 0)"
    else TOOL_OK[$name]=0; warn "$name sanity failed (exit $rc) -- will be skipped"; fi
}
log "sanity-checking each sandbox"
[ -n "$ZJAIL" ] && sanity z_jail "${ZJAIL_CMD[@]}" || TOOL_OK[z_jail]=0
command -v bwrap  >/dev/null 2>&1 && sanity bwrap  "${BWRAP_CMD[@]}"  || TOOL_OK[bwrap]=${TOOL_OK[bwrap]:-0}
{ [ -x "$NSJAIL_BIN" ] || command -v nsjail >/dev/null 2>&1; } && \
    sanity nsjail "${NSJAIL_CMD[@]}" || TOOL_OK[nsjail]=${TOOL_OK[nsjail]:-0}

# ------------------------------- statistics -----------------------------------
# reads numbers on stdin (one per line); prints: n mean sd ci min max median p95
stats_from_values() {
    sort -n | awk '
      {v[n++]=$1; sum+=$1}
      END{
        if(n==0){print "0 NA NA NA NA NA NA NA"; exit}
        mean=sum/n
        for(i=0;i<n;i++){d=v[i]-mean; ss+=d*d}
        sd=(n>1)?sqrt(ss/(n-1)):0
        ci=1.96*sd/sqrt(n)
        min=v[0]; max=v[n-1]
        if(n%2) med=v[int(n/2)]; else med=(v[n/2-1]+v[n/2])/2
        pi=int(0.95*(n-1)+0.5); p95=v[pi]
        printf "%d %.3f %.3f %.3f %.3f %.3f %.3f %.3f\n",n,mean,sd,ci,min,max,med,p95
      }'
}

run_bench() {
    # run_bench <name> <cmd...> ; writes csv, echoes report rows into globals
    local name="$1"; shift
    local csv="$CSVDIR/$name.csv"
    if [ "${TOOL_OK[$name]:-0}" -ne 1 ]; then
        warn "skipping $name (sanity failed / not available)"
        return 1
    fi
    log "benchmarking $name  (warmup=$WARMUP iters=$ITERS)"
    timeout -k 5 120 "$BUILD/harness" "$WARMUP" "$ITERS" "$@" > "$csv" 2>/dev/null
    local lat rss
    lat="$(tail -n +2 "$csv" | cut -d, -f2 | awk '{print $1/1e6}' | stats_from_values)"
    rss="$(tail -n +2 "$csv" | cut -d, -f3 | awk '{print $1/1024}' | stats_from_values)"
    # n mean sd ci min max med p95
    read -r LN LMEAN LSD LCI LMIN LMAX LMED LP95 <<<"$lat"
    read -r RN RMEAN RSD RCI RMIN RMAX RMED RP95 <<<"$rss"
    local ci_lo ci_hi
    ci_lo="$(awk "BEGIN{printf \"%.2f\", $LMEAN-$LCI}")"
    ci_hi="$(awk "BEGIN{printf \"%.2f\", $LMEAN+$LCI}")"
    LAT_ROW="| $name | $LN | $(printf '%.2f' "$LMEAN") +/- $(printf '%.2f' "$LSD") | [$ci_lo, $ci_hi] | $(printf '%.2f' "$LMED") | $(printf '%.2f' "$LP95") |"
    RSS_ROW="| $name | $RN | $(printf '%.2f' "$RMEAN") | $(printf '%.2f' "$RMAX") | $(printf '%.2f' "$RMED") | $(printf '%.2f' "$RP95") |"
    LAT_ROWS+=("$LAT_ROW")
    RSS_ROWS+=("$RSS_ROW")
    ok "$name: latency ${LMEAN}ms +/- ${LSD}  peak-RSS(mean) ${RMEAN}MiB max ${RMAX}MiB"
}

LAT_ROWS=(); RSS_ROWS=()
run_bench z_jail "${ZJAIL_CMD[@]}"
run_bench bwrap  "${BWRAP_CMD[@]}"
run_bench nsjail "${NSJAIL_CMD[@]}"

# ------------------------------ binary sizes ----------------------------------
size_of() { [ -f "$1" ] && stat -c%s "$1" || echo 0; }
ZJAIL_SIZE="$(size_of "$ZJAIL")"
ZJAIL_STRIPPED="$BUILD/z_jail.stripped"
cp "$ZJAIL" "$ZJAIL_STRIPPED" 2>/dev/null && strip "$ZJAIL_STRIPPED" 2>/dev/null
ZJAIL_STRIP_SIZE="$(size_of "$ZJAIL_STRIPPED")"
BWRAP_SIZE="$(size_of "$(command -v bwrap 2>/dev/null)")"
NSJAIL_SIZE="$(size_of "$NSJAIL_BIN")"
SLOC="$(cd "$REPO_DIR" && find src include -name '*.[ch]' -exec cat {} + 2>/dev/null | wc -l)"

# ---------------------------- functional tests --------------------------------
log "running the project functional test suite"
TESTS_OUT="$OUTDIR/functional_tests.txt"
( cd "$REPO_DIR" && timeout -k 5 180 bash tests/run_tests.sh ) > "$TESTS_OUT" 2>&1
TESTS_SUMMARY="$(grep -E '=== .* passed' "$TESTS_OUT" | tail -1)"
[ -n "$TESTS_SUMMARY" ] && ok "$TESTS_SUMMARY" || warn "test summary line not found (see $TESTS_OUT)"

# ------------------------------ system info -----------------------------------
KERNEL="$(uname -r)"
UNAME="$(uname -a)"
CPU="$(grep -m1 'model name' /proc/cpuinfo 2>/dev/null | cut -d: -f2- | sed 's/^ //')"
CORES="$(nproc 2>/dev/null)"
MEM="$(awk '/MemTotal/{printf "%.1f GiB", $2/1024/1024}' /proc/meminfo 2>/dev/null)"
GCCV="$(gcc --version 2>/dev/null | head -1)"
GLIBC="$(ldd --version 2>/dev/null | head -1)"
VIRT="$(command -v systemd-detect-virt >/dev/null 2>&1 && systemd-detect-virt 2>/dev/null || echo unknown)"
GOV="$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo n/a)"
ZJAIL_COMMIT="$(cd "$REPO_DIR" && git rev-parse --short HEAD 2>/dev/null || echo n/a)"

# ------------------------------- the report -----------------------------------
log "writing report -> $REPORT"
{
echo "# Z-Jail / TAC Native Benchmark Report"
echo
echo "Generated: $(date -u '+%Y-%m-%d %H:%M:%SZ')"
echo
echo "## 1. Environment"
echo
echo "| Field | Value |"
echo "|-------|-------|"
echo "| Distro | $DISTRO |"
echo "| Kernel | $KERNEL |"
echo "| uname | \`$UNAME\` |"
echo "| CPU | ${CPU:-unknown} |"
echo "| Cores (nproc) | ${CORES:-?} |"
echo "| Memory | ${MEM:-?} |"
echo "| Virtualization | $VIRT |"
echo "| CPU governor | $GOV |"
echo "| Compiler | ${GCCV:-?} |"
echo "| libc | ${GLIBC:-?} |"
echo "| Ran as root | $([ "$IS_ROOT" -eq 1 ] && echo yes || echo no) |"
echo "| Repo commit | $ZJAIL_COMMIT |"
echo
echo "## 2. Methodology"
echo
echo "One uniform methodology across all sandboxes. Every tool launches the"
echo "*same* freestanding, statically linked workload whose entire body is"
echo "\`exit_group(0)\` (no libc startup), so the process passes each sandbox's"
echo "policy and exits cleanly, isolating setup cost from workload cost."
echo "For each tool: **$WARMUP warm-up runs**, then **$ITERS measured samples**."
echo "Latency = wall-clock (\`CLOCK_MONOTONIC\`) around fork->wait4."
echo "Peak RSS = \`ru_maxrss\` from \`wait4(2)\` (KiB), reported in MiB."
echo "Workload size: $(size_of "$BUILD/workload") bytes."
echo
echo "Exact commands measured:"
echo '```'
echo "z_jail : ${ZJAIL_CMD[*]}"
echo "bwrap  : ${BWRAP_CMD[*]}"
echo "nsjail : ${NSJAIL_CMD[*]}"
echo '```'
echo
echo "## 3. Setup latency (ms)"
echo
echo "| Tool | n | mean +/- sd | 95% CI | median | p95 |"
echo "|------|---|-------------|--------|--------|-----|"
for r in "${LAT_ROWS[@]}"; do echo "$r"; done
echo
echo "## 4. Peak resident set (MiB)"
echo
echo "| Tool | n | mean | max | median | p95 |"
echo "|------|---|------|-----|--------|-----|"
for r in "${RSS_ROWS[@]}"; do echo "$r"; done
echo
echo "## 5. Binary sizes"
echo
echo "| Binary | Bytes | ~KiB |"
echo "|--------|-------|------|"
echo "| z_jail (unstripped) | $ZJAIL_SIZE | $(awk "BEGIN{printf \"%.1f\", $ZJAIL_SIZE/1024}") |"
echo "| z_jail (stripped)   | $ZJAIL_STRIP_SIZE | $(awk "BEGIN{printf \"%.1f\", $ZJAIL_STRIP_SIZE/1024}") |"
echo "| bwrap               | $BWRAP_SIZE | $(awk "BEGIN{printf \"%.1f\", $BWRAP_SIZE/1024}") |"
echo "| nsjail              | $NSJAIL_SIZE | $(awk "BEGIN{printf \"%.1f\", $NSJAIL_SIZE/1024}") |"
echo
echo "Source lines (src + include, .c/.h): **$SLOC**"
echo
echo "## 6. Functional test suite"
echo
echo "\`\`\`"
cat "$TESTS_OUT"
echo "\`\`\`"
echo
echo "Summary: **${TESTS_SUMMARY:-not found}**"
echo
echo "## 7. Notes & caveats"
echo
echo "- Numbers are single-host; report them as relative, not absolute."
echo "- bwrap applies no default seccomp filter, so it performs less setup work."
echo "- gVisor and Firecracker are intentionally excluded (user-space-kernel and"
echo "  microVM cold-boot are different metrics than fork-to-exec setup)."
echo "- Raw per-sample data is in \`csv/\` alongside this report."
if [ "$GOV" != "performance" ] && [ "$GOV" != "n/a" ]; then
echo "- CPU governor is '$GOV'; for the tightest numbers set 'performance'"
echo "  (\`sudo cpupower frequency-set -g performance\`) and re-run."
fi
} > "$REPORT"

# ------------------------------- wrap up --------------------------------------
echo
ok "DONE."
echo "-------------------------------------------------------------------"
echo "Report : $REPORT"
echo "CSVs   : $CSVDIR/"
echo "Tests  : $TESTS_OUT"
echo "-------------------------------------------------------------------"
echo "Send me the whole '$OUTDIR' directory (or at least REPORT.md + csv/)."
