#include "z_jail.h"
#include <time.h>
#include <sys/ioctl.h>

/* Inode append-only flag (the "a" in chattr +a). Defined locally so we do not
 * have to pull in <linux/fs.h>, which can clash with libc mount/stat headers.
 * The numeric values are stable kernel ABI. */
#ifndef FS_APPEND_FL
#define FS_APPEND_FL 0x00000020
#endif
#ifndef FS_IOC_GETFLAGS
#  ifdef _IOR
#    define FS_IOC_GETFLAGS _IOR('f', 1, long)
#  else
#    define FS_IOC_GETFLAGS 0x80086601UL
#  endif
#endif
#ifndef FS_IOC_SETFLAGS
#  ifdef _IOW
#    define FS_IOC_SETFLAGS _IOW('f', 2, long)
#  else
#    define FS_IOC_SETFLAGS 0x40086602UL
#  endif
#endif

/* openat2(2) with RESOLVE_NO_SYMLINKS refuses to traverse a symlink in ANY
 * path component, not just the final one that O_NOFOLLOW guards. Available on
 * Linux >= 5.6; we fall back to an O_NOFOLLOW open() when it is missing. */
#if defined(__has_include)
#  if __has_include(<linux/openat2.h>)
#    include <linux/openat2.h>
#    define AXIOM_HAVE_OPENAT2_HDR 1
#  endif
#endif
#ifndef RESOLVE_NO_SYMLINKS
#define RESOLVE_NO_SYMLINKS 0x04ULL
#endif
#if defined(SYS_openat2)
#  ifndef AXIOM_HAVE_OPENAT2_HDR
struct open_how { uint64_t flags; uint64_t mode; uint64_t resolve; };
#  endif
#  define AXIOM_USE_OPENAT2 1
#endif

static const char AXIOM_GENESIS_HASH[65] =
    "0000000000000000000000000000000000000000000000000000000000000000";

/* Read the last complete line of an existing audit file and record its
 * BLAKE2b-256 hex digest as prev_hash. This anchors the hash chain to the
 * current tail so the next record we append commits to it. Any failure (no
 * file, empty file, unreadable) leaves the genesis (all-zero) anchor in place. */
static void audit_load_prev_hash(axiom_audit *audit)
{
    static uint8_t buf[65536];
    axiom_blake2b_ctx ctx;
    uint8_t digest[AXIOM_BLAKE2B_OUT_LEN];
    off_t size, want;
    ssize_t rd;
    size_t start, end;
    int fd;

    memcpy(audit->prev_hash_hex, AXIOM_GENESIS_HASH, sizeof(AXIOM_GENESIS_HASH));

    fd = open(audit->path, O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    if (fd < 0) return;

    size = lseek(fd, 0, SEEK_END);
    if (size <= 0) { close(fd); return; }

    want = (size < (off_t)sizeof(buf)) ? size : (off_t)sizeof(buf);
    if (lseek(fd, size - want, SEEK_SET) == (off_t)-1) { close(fd); return; }
    rd = read(fd, buf, (size_t)want);
    close(fd);
    if (rd <= 0) return;

    end = (size_t)rd;
    while (end > 0 && (buf[end-1] == '\n' || buf[end-1] == '\r')) end--;
    if (end == 0) return;
    start = end;
    while (start > 0 && buf[start-1] != '\n') start--;

    axiom_blake2b_init(&ctx);
    axiom_blake2b_update(&ctx, buf + start, end - start);
    axiom_blake2b_final(&ctx, digest);
    axiom_hex_encode(digest, AXIOM_BLAKE2B_OUT_LEN, audit->prev_hash_hex);
}

static int audit_open_fd(const char *path)
{
    int fd;
#ifdef AXIOM_USE_OPENAT2
    struct open_how how;
    memset(&how, 0, sizeof(how));
    how.flags   = (uint64_t)(O_WRONLY|O_CREAT|O_APPEND|O_CLOEXEC);
    how.mode    = 0600;
    how.resolve = RESOLVE_NO_SYMLINKS;
    fd = (int)syscall(SYS_openat2, AT_FDCWD, path, &how, sizeof(how));
    if (fd >= 0) return fd;
    if (errno != ENOSYS && errno != EINVAL && errno != EPERM)
        return -1;
    /* Kernel/seccomp does not support openat2: fall back below. */
#endif
    fd = open(path, O_WRONLY|O_CREAT|O_APPEND|O_NOFOLLOW|O_CLOEXEC, 0600);
    return fd;
}

/* Make the audit file append-only at the filesystem level (ext4/xfs/btrfs).
 * Once set, even the owner cannot truncate or overwrite existing bytes; only a
 * process holding CAP_LINUX_IMMUTABLE can clear the flag. This is best-effort:
 * on filesystems without flag support, or without the capability, it degrades
 * gracefully so auditing still works (without the tamper resistance). */
static void audit_set_append_only(int fd)
{
    int flags = 0;
    if (ioctl(fd, (unsigned long)FS_IOC_GETFLAGS, &flags) != 0)
        return;
    if (flags & FS_APPEND_FL)
        return;
    flags |= FS_APPEND_FL;
    if (ioctl(fd, (unsigned long)FS_IOC_SETFLAGS, &flags) != 0)
        axiom_log(LOG_WARN,
            "audit: append-only (chattr +a) not applied to %s: %s\n",
            "audit file", strerror(errno));
}

axiom_audit *axiom_audit_open(const char *path)
{
    axiom_audit *audit;
    const char *slash;

    audit = calloc(1, sizeof(*audit));
    if (!audit) return NULL;

    audit->path = strdup(path);
    if (!audit->path) { free(audit); return NULL; }

    slash = strrchr(path, '/');
    if (slash) {
        char *dir = strndup(path, (size_t)(slash - path));
        if (dir) { mkdir(dir, 0700); free(dir); }
    }

    audit_load_prev_hash(audit);

    audit->fd = audit_open_fd(path);
    if (audit->fd < 0) { free(audit->path); free(audit); return NULL; }

    fchmod(audit->fd, 0600);
    audit_set_append_only(audit->fd);
    audit->start_time = time(NULL);
    return audit;
}

int axiom_audit_write(axiom_audit *audit, const char *table, const char *json)
{
    char buf[65536];
    axiom_blake2b_ctx ctx;
    uint8_t digest[AXIOM_BLAKE2B_OUT_LEN];
    int n;
    ssize_t w;
    (void)table;

    n = snprintf(buf, sizeof(buf), "%s\n", json);
    if (n < 0 || (size_t)n >= sizeof(buf)) return -1;

    w = write(audit->fd, buf, (size_t)n);
    if (w < 0 || (size_t)w != (size_t)n) return -1;

    /* Advance the chain: the anchor for the NEXT record is the digest of this
     * record's JSON line, excluding the trailing newline. A verifier computes
     * the identical value, so any tampering with this line is detectable. */
    axiom_blake2b_init(&ctx);
    axiom_blake2b_update(&ctx, buf, (size_t)n - 1);
    axiom_blake2b_final(&ctx, digest);
    axiom_hex_encode(digest, AXIOM_BLAKE2B_OUT_LEN, audit->prev_hash_hex);
    return 0;
}

int axiom_audit_close(axiom_audit *audit)
{
    if (!audit) return 0;
    if (audit->fd >= 0) close(audit->fd);
    free(audit->path);
    free(audit);
    return 0;
}

void axiom_audit_timestamp(char *buf, size_t size, long long ns) {
    struct tm tm;
    time_t sec = (time_t)(ns / 1000000000LL);
    localtime_r(&sec, &tm);
    strftime(buf, size, "%Y-%m-%dT%H:%M:%S%z", &tm);
}
