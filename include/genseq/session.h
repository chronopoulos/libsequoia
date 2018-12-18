#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>
#include <jack/jack.h>

#include "sequence.h"

// (config parameters)
#define MAX_NSEQ 256
#define RINGBUFFER_LENGTH 16
#define DEFAULT_BPM 120.00

// (constants for bpm translation)
#define STEPS_PER_BEAT 4
#define SECONDS_PER_MINUTE 60

enum _session_param {SESSION_GO, SESSION_BPM};

struct _session_ctrl_msg {

    enum _session_param param;

    // parameter-dependent value fields
    int vi;
    float vf;
    bool vb;

};

struct gs_session_data {

    float bpm; // beats per minute
    int tps; // ticks per step
    int fpt; // frames per tick
    bool go;

    jack_client_t *jack_client;
    jack_port_t *jack_port_out;
    jack_nframes_t sr; // sample rate
    jack_nframes_t bs; // buffer size

    struct gs_sequence_data *seqs[MAX_NSEQ];
    int nseqs;

    jack_ringbuffer_t *rb;

    jack_nframes_t frame;

};

void gs_session_init(struct gs_session_data*, char*, int);
void gs_session_start(struct gs_session_data*);
void gs_session_stop(struct gs_session_data*);
void gs_session_set_bpm(struct gs_session_data*, float);
void gs_session_add_sequence(struct gs_session_data*, struct gs_sequence_data*);
void gs_session_wait(struct gs_session_data*);

#endif
