#pragma once

typedef enum {
    AXIOM_VERDICT_DETERMINISTIC = 0,
    AXIOM_VERDICT_REJECT       = 1,
    AXIOM_VERDICT_UNCERTAIN    = 2
} axiom_verdict;

typedef double axiom_weight;

typedef int (*axiom_evidence_fn)(void *ctx,
    const char *target, axiom_weight *out_weight,
    axiom_verdict *out_verdict);

typedef struct axiom_evidence_chain {
    axiom_weight        total;
    axiom_verdict       verdict;
    const char         *fail_reason;
    struct axiom_evidence_link {
        const char              *id;
        axiom_weight             weight;
        axiom_verdict            verdict;
        const char              *target;
        struct axiom_evidence_link *next;
    } *head;
} axiom_evidence_chain;

int axiom_truthimatics_eval(axiom_evidence_chain *chain,
    const char *target);
void axiom_truthimatics_cleanup(axiom_evidence_chain *chain);
