#include "z_jail.h"
#include <time.h>

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

    audit->fd = open(path, O_WRONLY|O_CREAT|O_APPEND|O_NOFOLLOW|O_CLOEXEC, 0600);
    if (audit->fd < 0) { free(audit->path); free(audit); return NULL; }

    fchmod(audit->fd, 0600);
    audit->start_time = time(NULL);
    return audit;
}

int axiom_audit_write(axiom_audit *audit, const char *table, const char *json)
{
    char buf[65536];
    int n = snprintf(buf, sizeof(buf), "%s\n", json);
    if (n < 0 || (size_t)n >= sizeof(buf)) return -1;
    ssize_t w = write(audit->fd, buf, (size_t)n);
    (void)table;
    return (w < 0 || (size_t)w != (size_t)n) ? -1 : 0;
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
