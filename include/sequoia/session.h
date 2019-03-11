/*

    Copyright 2018, Chris Chronopoulos

    This file is part of libsequoia.

    libsequoia is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libsequoia is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libsequoia.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>
#include <jack/jack.h>
#include <jack/midiport.h>

#include "sequence.h"
#include "port.h"

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
int sq_session_register_port(sq_session_t *, sq_port_t*);
void sq_session_start(sq_session_t*);
void sq_session_stop(sq_session_t*);

void sq_session_set_bpm(sq_session_t*, float);
void _session_set_bpm_now(sq_session_t*, float);

void sq_session_add_sequence(sq_session_t*, sq_sequence_t*);
void _session_add_sequence_now(sq_session_t*, sq_sequence_t*);

void sq_session_rm_sequence(sq_session_t*, sq_sequence_t*);
void _session_rm_sequence_now(sq_session_t*, sq_sequence_t*);

char *sq_session_get_name(sq_session_t*);

int sq_session_get_tps(sq_session_t*);
int sq_session_get_bpm(sq_session_t*);

#endif
