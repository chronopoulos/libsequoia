#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <pthread.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

typedef jack_midi_data_t midi_packet[3]; // equivalent to 3 unsigned chars

struct zig_sequence_data {

    int nsteps;
    int tps;
    jack_nframes_t fpt;

    struct zig_trigger_data *trigs;

    int nticks;
    midi_packet *ticks;

    int tick;
    jack_nframes_t frame;

};

void zig_sequence_init(struct zig_sequence_data*, int, int, int);
void zig_sequence_set_raw_tick(struct zig_sequence_data*, int, midi_packet*);
void zig_sequence_set_trig(struct zig_sequence_data*, int, struct zig_trigger_data*);
void zig_sequence_clear_trig(struct zig_sequence_data*, int);
void zig_sequence_process(struct zig_sequence_data*, jack_nframes_t, void*);

#endif
