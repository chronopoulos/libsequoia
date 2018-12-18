#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <pthread.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

#define MAX_NAME_LENGTH 255

typedef jack_midi_data_t midi_packet[3]; // equivalent to 3 unsigned chars

enum _sequence_param {SEQUENCE_TRANSPOSE};

struct _sequence_ctrl_msg {

    enum _sequence_param param;

    // parameter-dependent value fields
    int vi;
    float vf;
    bool vb;
    struct gs_trigger_data *vp;

};

struct gs_sequence_data {

    // these are accessible from the UI thread
    char name[MAX_NAME_LENGTH + 1];
    int transpose;
    struct gs_trigger_data *trigs;
    midi_packet *ticks;
    int tick;

    int nsteps;
    int tps;
    int nticks;

    jack_ringbuffer_t *rb;

};

void gs_sequence_init(struct gs_sequence_data*, int, int);
void gs_sequence_set_name(struct gs_sequence_data*, const char*);
void gs_sequence_set_raw_tick(struct gs_sequence_data*, int, midi_packet*);
void gs_sequence_set_trig(struct gs_sequence_data*, int, struct gs_trigger_data*);
void gs_sequence_clear_trig(struct gs_sequence_data*, int);
void gs_sequence_tick(struct gs_sequence_data*, void*, jack_nframes_t);
void gs_sequence_set_transpose(struct gs_sequence_data*, int);

#endif
