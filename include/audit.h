#pragma once

typedef struct {
    char     *path;
    int       fd;
    time_t    start_time;
    /* Rolling hash chain: BLAKE2b-256 (hex) of the previous audit record's
     * JSON line, or 64 zeros for the genesis (first) record. Each record
     * commits to its predecessor so deletion, reordering or in-place edits
     * are detectable by an external verifier. */
    char      prev_hash_hex[65];
} axiom_audit;

axiom_audit *axiom_audit_open(const char *path);
int axiom_audit_write(axiom_audit *audit,
    const char *table, const char *json);
int axiom_audit_close(axiom_audit *audit);
