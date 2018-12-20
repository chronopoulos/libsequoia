#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>
#include <jack/jack.h>
#include <jack/midiport.h>

#include "sequence.h"

// (config parameters)
#define MAX_NSEQ 256
#define RINGBUFFER_LENGTH 16
#define DEFAULT_BPM 120.00

// (constants for bpm translation)
#define STEPS_PER_BEAT 4
#define SECONDS_PER_MINUTE 60

enum _session_param {SESSION_GO, SESSION_BPM, SESSION_ADD_SEQ, SESSION_RM_SEQ};

struct _session_ctrl_msg {

    enum _session_param param;

    // parameter-dependent value fields
    int vi;
    float vf;
    bool vb;
    struct sq_sequence_data *vp;

};

struct sq_session_data {

    // both audio and UI thread
    float bpm; // beats per minute
    bool go;
    struct sq_sequence_data *seqs[MAX_NSEQ];
    int nseqs;

    // UI only
    bool is_playing;

    // constant
    int tps; // ticks per step
    int fpt; // frames per tick
    jack_client_t *jack_client;
    jack_nframes_t sr; // sample rate
    jack_nframes_t bs; // buffer size
    jack_ringbuffer_t *rb;

    // audio only
    jack_nframes_t frame;

};

void sq_session_init(struct sq_session_data*, const char*, int);
jack_port_t *sq_session_create_outport(struct sq_session_data *, const char*);
void sq_session_start(struct sq_session_data*);
void sq_session_stop(struct sq_session_data*);

void sq_session_set_bpm(struct sq_session_data*, float);
void _session_set_bpm_now(struct sq_session_data*, float);

void sq_session_add_sequence(struct sq_session_data*, struct sq_sequence_data*);
void _session_add_sequence_now(struct sq_session_data*, struct sq_sequence_data*);

void sq_session_rm_sequence(struct sq_session_data*, struct sq_sequence_data*);
void _session_rm_sequence_now(struct sq_session_data*, struct sq_sequence_data*);

#endif
