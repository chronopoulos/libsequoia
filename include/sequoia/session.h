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

typedef struct {

    enum _session_param param;

    // parameter-dependent value fields
    int vi;
    float vf;
    bool vb;
    sq_sequence_t *vp;

} _session_ctrl_msg_t ;

typedef struct {

    // both audio and UI thread
    float bpm; // beats per minute
    bool go;
    sq_sequence_t *seqs[MAX_NSEQ];
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

} sq_session_t;

void sq_session_init(sq_session_t*, const char*, int);
jack_port_t *sq_session_create_outport(sq_session_t *, const char*);
void sq_session_start(sq_session_t*);
void sq_session_stop(sq_session_t*);

void sq_session_set_bpm(sq_session_t*, float);
void _session_set_bpm_now(sq_session_t*, float);

void sq_session_add_sequence(sq_session_t*, sq_sequence_t*);
void _session_add_sequence_now(sq_session_t*, sq_sequence_t*);

void sq_session_rm_sequence(sq_session_t*, sq_sequence_t*);
void _session_rm_sequence_now(sq_session_t*, sq_sequence_t*);

#endif
