#pragma once

typedef struct {
    char     *path;
    int       fd;
    time_t    start_time;
} axiom_audit;

axiom_audit *axiom_audit_open(const char *path);
int axiom_audit_write(axiom_audit *audit,
    const char *table, const char *json);
int axiom_audit_close(axiom_audit *audit);
