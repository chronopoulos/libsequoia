#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <pthread.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

#define MAX_NAME_LENGTH 255

typedef jack_midi_data_t midi_packet[3]; // equivalent to 3 unsigned chars

struct gs_sequence_data {

    int nsteps;
    int tps;
    jack_nframes_t fpt;

    char name[MAX_NAME_LENGTH + 1];
    int transpose;

    struct gs_trigger_data *trigs;

    int nticks;
    midi_packet *ticks;

    int tick;
    jack_nframes_t frame;

};

void gs_sequence_init(struct gs_sequence_data*, int, int, int);
void gs_sequence_set_name(struct gs_sequence_data*, const char*);
void gs_sequence_set_raw_tick(struct gs_sequence_data*, int, midi_packet*);
void gs_sequence_set_trig(struct gs_sequence_data*, int, struct gs_trigger_data*);
void gs_sequence_clear_trig(struct gs_sequence_data*, int);
void gs_sequence_process(struct gs_sequence_data*, jack_nframes_t, void*);

#endif
