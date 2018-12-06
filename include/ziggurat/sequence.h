#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <jack/ringbuffer.h>

struct zig_sequence_data {

    int *trigs;
    int length;

    int playhead;

    jack_ringbuffer_t *rbi;
    jack_ringbuffer_t *rbo;

};

void zig_sequence_init(struct zig_sequence_data*, int);
void zig_sequence_set_rbi(struct zig_sequence_data*, jack_ringbuffer_t*);
void zig_sequence_set_rbo(struct zig_sequence_data*, jack_ringbuffer_t*);
void zig_sequence_set_trig(struct zig_sequence_data*, int, int);
void zig_sequence_tick(struct zig_sequence_data*);

#endif
