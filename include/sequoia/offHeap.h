#ifndef OFFHEAP_H
#define OFFHEAP_H

#include <stdint.h>
#include <stddef.h>

#include "sequoia/midiEvent.h"

typedef struct offNode {
    midiEvent mev;
    struct offNode *next;
} offNode_t;

typedef struct {
    size_t n;
    offNode_t *slots;
    offNode_t **avail;  // array of pointers
    ssize_t ravail, wavail, savail; // read, write, size (operating as a queue)
} offHeap_t;

// constructor and destructor
offHeap_t *offHeap_new(size_t);
void offHeap_delete(offHeap_t*);

// methods
offNode_t *offHeap_alloc(offHeap_t*);
void offHeap_free(offHeap_t *offHeap, offNode_t *offNode);

#endif
