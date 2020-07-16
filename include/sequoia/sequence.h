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

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <pthread.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>
#include <json/json.h>

#include "trigger.h"
#include "outport.h"

#define MAX_SEQ_NAME_LEN 255

typedef jack_midi_data_t midi_packet[3]; // equivalent to 3 unsigned chars

enum _sequence_param {SEQUENCE_SET_TRIG, SEQUENCE_CLEAR_TRIG, SEQUENCE_TRANSPOSE, SEQUENCE_PH,
                        SEQUENCE_DIV, SEQUENCE_MUTE, SEQUENCE_FIRST, SEQUENCE_LAST};

typedef struct {

    enum _sequence_param param;

    // parameter-dependent value fields
    int vi;
    float vf;
    bool vb;
    sq_trigger_t *vp;

} _sequence_ctrl_msg_t;

typedef struct {

    bool playhead_new;
    int playhead;

    bool first_new;
    int first;

    bool last_new;
    int last;

    bool transpose_new;
    int transpose;

    bool clockdivide_new;
    int clockdivide;

    bool mute_new;
    bool mute;

} sq_sequence_noti_t;

typedef struct {

    // these are touched by both the audio and the UI thread
    char name[MAX_SEQ_NAME_LEN + 1];
    int transpose;
    sq_trigger_t *trigs;
    sq_trigger_t **microgrid;
    midi_packet *buf_off;
    int ridx_off;

    // TBD
    sq_outport_t *outport;

    // this is only touched by UI
    bool is_playing;

    int tps; // ticks per step
    int nsteps;
    int nticks;
    int tick;

    jack_ringbuffer_t *rb;

    int div, idiv;
    bool mute;

    int first, last;

    sq_sequence_noti_t noti;
    bool noti_enable;

} sq_sequence_t;

void sq_sequence_init(sq_sequence_t*, int, int);
void sq_sequence_noti_init(sq_sequence_noti_t*);

void _sequence_tick(sq_sequence_t*, jack_nframes_t);
void _sequence_serve_off_buffer(sq_sequence_t*, jack_nframes_t);
void _sequence_reset_now(sq_sequence_t*);

void sq_sequence_set_name(sq_sequence_t*, const char*);
void sq_sequence_set_outport(sq_sequence_t*, sq_outport_t*);

void sq_sequence_set_trig(sq_sequence_t*, int, sq_trigger_t*);
void _sequence_set_trig_now(sq_sequence_t*, int, sq_trigger_t*);

void sq_sequence_clear_trig(sq_sequence_t*, int);
void _sequence_clear_trig_now(sq_sequence_t*, int);

void sq_sequence_set_transpose(sq_sequence_t*, int);
void _sequence_set_transpose_now(sq_sequence_t*, int);

void sq_sequence_set_playhead(sq_sequence_t*, int);
void _sequence_set_playhead_now(sq_sequence_t*, int);

void sq_sequence_set_first(sq_sequence_t*, int);
void _sequence_set_first_now(sq_sequence_t*, int);

void sq_sequence_set_last(sq_sequence_t*, int);
void _sequence_set_last_now(sq_sequence_t*, int);

void sq_sequence_set_clockdivide(sq_sequence_t*, int);
void _sequence_set_clockdivide_now(sq_sequence_t*, int);

void sq_sequence_set_mute(sq_sequence_t*, bool);
void _sequence_set_mute_now(sq_sequence_t*, bool);

void sq_sequence_pprint(sq_sequence_t*);

void sq_sequence_set_notifications(sq_sequence_t*, bool);
bool sq_sequence_read_new_playhead(sq_sequence_t*, int*);
bool sq_sequence_read_new_first(sq_sequence_t*, int*);
bool sq_sequence_read_new_last(sq_sequence_t*, int*);
bool sq_sequence_read_new_transpose(sq_sequence_t*, int*);
bool sq_sequence_read_new_clockdivide(sq_sequence_t*, int*);
bool sq_sequence_read_new_mute(sq_sequence_t*, bool*);

int sq_sequence_get_nsteps(sq_sequence_t*);
int sq_sequence_get_tps(sq_sequence_t*);
bool sq_sequence_get_mute(sq_sequence_t*);
int sq_sequence_get_transpose(sq_sequence_t*);
int sq_sequence_get_clockdivide(sq_sequence_t*);
int sq_sequence_get_first(sq_sequence_t*);
int sq_sequence_get_last(sq_sequence_t*);

json_object *sq_sequence_get_json(sq_sequence_t*);
sq_sequence_t *sq_sequence_malloc_from_json(json_object*);

#endif
