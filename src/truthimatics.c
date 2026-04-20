#include "z_jail.h"

int axiom_evidence_add_fn(axiom_evidence_chain *chain,
    const char *id, axiom_weight weight,
    axiom_evidence_fn fn, void *ctx, const char *target)
{
    struct axiom_evidence_link *link;
    axiom_weight out_weight;
    axiom_verdict out_verdict;
    int rc;

    if (!chain || !fn)
        return -1;

    rc = fn(ctx, target, &out_weight, &out_verdict);
    if (rc < 0)
        return rc;

    link = malloc(sizeof(*link));
    if (!link)
        return -1;

    link->id      = id;
    link->weight  = out_weight;
    link->verdict = out_verdict;
    link->target  = target;
    link->next    = chain->head;
    chain->head   = link;

    chain->total += out_weight;

    if (out_verdict == AXIOM_VERDICT_REJECT)
        chain->verdict = AXIOM_VERDICT_REJECT;

    return 0;
}
