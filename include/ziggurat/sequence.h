#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <pthread.h>
#include <jack/ringbuffer.h>

struct zig_sequence_data {

    int nsteps;
    int tps;
    jack_nframes_t fpt;

    int nticks;
    int *ticks;

    int tick;
    jack_nframes_t frame;

};

void zig_sequence_init(struct zig_sequence_data*, int, int, int);
void zig_sequence_set_step(struct zig_sequence_data*, int, int);
void zig_sequence_process(struct zig_sequence_data*, jack_nframes_t, void*);

#endif
