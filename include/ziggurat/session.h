#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>
#include <jack/jack.h>

#include "sequence.h"

#define ZIGGURAT_MAX_NSEQ 256

// for bpm translation:
#define STEPS_PER_BEAT 4
#define SECONDS_PER_MINUTE 60

struct zig_session_data {

    int bpm; // beats per minute
    int tps; // ticks per step
    int fpt; // frames per tick
    bool go;

    jack_client_t *jack_client;
    jack_port_t *jack_port_out;
    jack_nframes_t sr; // sample rate
    jack_nframes_t bs; // buffer size

    struct zig_sequence_data *seqs[ZIGGURAT_MAX_NSEQ];
    int nseqs;

};

void zig_session_init(struct zig_session_data*, char*, int, int);
void zig_session_start(struct zig_session_data*);
void zig_session_stop(struct zig_session_data*);
void zig_session_add_sequence(struct zig_session_data*, struct zig_sequence_data*);
void zig_session_wait(struct zig_session_data*);

#endif
