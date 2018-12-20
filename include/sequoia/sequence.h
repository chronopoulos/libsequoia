#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <pthread.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

#define MAX_NAME_LENGTH 255

typedef jack_midi_data_t midi_packet[3]; // equivalent to 3 unsigned chars

enum _sequence_param {SEQUENCE_TRANSPOSE, SEQUENCE_PH};

struct _sequence_ctrl_msg {

    enum _sequence_param param;

    // parameter-dependent value fields
    int vi;
    float vf;
    bool vb;
    struct sq_trigger_data *vp;

};

struct sq_sequence_data {

    // these are accessible (indirectly) from the UI thread
    char name[MAX_NAME_LENGTH + 1];
    int transpose;
    struct sq_trigger_data *trigs;
    struct sq_trigger_data **microgrid;
    int ph;
    midi_packet *buf_off;
    int ridx_off;

    int nsteps;
    int tps;
    int nticks;

    jack_ringbuffer_t *rb;

};

void sq_sequence_init(struct sq_sequence_data*, int, int);
void sq_sequence_set_name(struct sq_sequence_data*, const char*);
void sq_sequence_set_trig(struct sq_sequence_data*, int, struct sq_trigger_data*);
void sq_sequence_clear_trig(struct sq_sequence_data*, int);
void sq_sequence_tick(struct sq_sequence_data*, void*, jack_nframes_t);
void sq_sequence_set_transpose(struct sq_sequence_data*, int);
void sq_sequence_set_playhead(struct sq_sequence_data*, int);

#endif
