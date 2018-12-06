#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>
#include <pthread.h>
#include <jack/jack.h>
#include <jack/ringbuffer.h>

#include "sequence.h"

#define RINGBUFFER_SIZE		1024*3
#define PERIOD_SEC_DEFAULT 0.125 // 120 bpm
#define ZIGGURAT_MAX_NSEQ 256
#define ZIGGURAT_TYPE_JACK 0

struct zig_session_data {

    bool go;
    double period_sec;

    jack_client_t *jack_client;
    jack_port_t *jack_port_out;
    jack_ringbuffer_t *rbo;

    pthread_mutex_t  mtx_go;
    pthread_t thread_metro;

    struct zig_sequence_data *seqs[ZIGGURAT_MAX_NSEQ];
    int nseqs;


};

void zig_session_init(struct zig_session_data*, int);
void zig_session_start(struct zig_session_data*);
void zig_session_stop(struct zig_session_data*);
void zig_session_add_sequence(struct zig_session_data*, struct zig_sequence_data*);
void zig_session_wait(struct zig_session_data*);

#endif
