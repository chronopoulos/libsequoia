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
#include <json-c/json.h> 

#include "sequence.h"
#include "outport.h"
#include "inport.h"
#include "offHeap.h"

#define SESSION_MAX_NSEQ 256
#define SESSION_MAX_NINPORTS 16
#define SESSION_MAX_NOUTPORTS 16

typedef struct {

    // both audio and UI thread
    float bpm; // beats per minute
    bool go; sq_sequence_t *seqs[SESSION_MAX_NSEQ];
    int nseqs;

    // UI only
    bool is_playing;

    // constant
    int fps; // frames per step

    jack_client_t *jack_client;
    jack_nframes_t sr; // sample rate
    jack_nframes_t bs; // buffer size
    jack_ringbuffer_t *rb;

    // audio only
    jack_nframes_t frame;

    // ports
    sq_inport_t *inports[SESSION_MAX_NINPORTS];
    sq_outport_t *outports[SESSION_MAX_NOUTPORTS];
    int ninports, noutports;

    // note-off buffer
    offNode_t **buf_off;    // array of pointers
    size_t len_off;
    size_t idx_off;
    offHeap_t *offHeap;

} sq_session_t;

sq_session_t *sq_session_new(const char*);
int sq_session_register_outport(sq_session_t *, sq_outport_t*);
int sq_session_register_inport(sq_session_t *, sq_inport_t*);
void sq_session_start(sq_session_t*);
void sq_session_stop(sq_session_t*);

sq_sequence_t *sq_session_get_sequence_from_name(sq_session_t*, const char*);
sq_inport_t *sq_session_get_inport_from_name(sq_session_t*, const char*);
sq_outport_t *sq_session_get_outport_from_name(sq_session_t*, const char*);

void sq_session_set_bpm(sq_session_t*, float);
void sq_session_add_sequence(sq_session_t*, sq_sequence_t*);
void sq_session_rm_sequence(sq_session_t*, sq_sequence_t*);
char *sq_session_get_name(sq_session_t*);
float sq_session_get_bpm(sq_session_t*);

void sq_session_save(sq_session_t*, const char*);
sq_session_t *sq_session_load(const char*);
void sq_session_teardown(sq_session_t*);

#endif
