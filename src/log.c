#include "z_jail.h"
#include <stdarg.h>
#include <unistd.h>

int axiom_log_level = LOG_INFO;

void axiom_log(int level, const char *fmt, ...)
{
    char buf[4096];
    va_list ap;
    int n;

    if (level > axiom_log_level)
        return;

    n = snprintf(buf, sizeof(buf), "[z-jail] ");
    if (n < 0 || n >= (int)sizeof(buf))
        return;

    va_start(ap, fmt);
    vsnprintf(buf + n, sizeof(buf) - (size_t)n, fmt, ap);
    va_end(ap);

    write(STDERR_FILENO, buf, strlen(buf));
}
