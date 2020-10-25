// usage:
//  offNode_t *offNode = offHeap_alloc(offHeap)
//  offNode->mev = mev;
//  offNode->next = NULL;
//  prevNode->next = offNode;

#include <stdlib.h>
#include <stdio.h>
#include "sequoia/offHeap.h"

offHeap_t *offHeap_new(size_t n) {

    offHeap_t *offHeap;

    offHeap = malloc(sizeof(offHeap_t));

    offHeap->n = n;
    offHeap->slots = malloc(offHeap->n * sizeof(offNode_t));
    offHeap->avail = malloc(offHeap->n * sizeof(offNode_t*));

    // initialize the avail queue with every slot
    for (size_t i=0; i<offHeap->n; i++) {
        offHeap->avail[i] = offHeap->slots + i;
    }
    offHeap->ravail = 0;
    offHeap->wavail = 0;
    offHeap->savail = offHeap->n;

    return offHeap;

}

void offHeap_delete(offHeap_t *offHeap) {

    free(offHeap->avail);
    free(offHeap->slots);
    free(offHeap);

}

offNode_t *offHeap_alloc(offHeap_t *offHeap) {

    // check if avail queue is empty
    if (offHeap->savail == 0) {
        fprintf(stderr, "offHeap_alloc: heap is full (queue is empty)\n");
        return NULL;
    }

    // pead a node pointer off the avail queue
    offNode_t *offNode = offHeap->avail[offHeap->ravail];

    // handle queue state
    offHeap->ravail++;
    if (offHeap->ravail == offHeap->n) offHeap->ravail = 0;
    offHeap->savail--;

    return offNode;
    
}

void offHeap_free(offHeap_t *offHeap, offNode_t *offNode) {

    // check if avail queue is full
    // (this shouldn't happen, unless you try to double-free something)
    if (offHeap->savail == offHeap->n) {
        fprintf(stderr, "offHeap_alloc: heap is empty (queue is full)\n");
        return;
    }

    // write the node pointer to the avail queue
    offHeap->avail[offHeap->wavail] = offNode;

    // handle queue state
    offHeap->wavail++;
    if (offHeap->wavail == offHeap->n) offHeap->wavail = 0;
    offHeap->savail++;

}

