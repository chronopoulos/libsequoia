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

#ifndef SEQUOIA_H
#define SEQUOIA_H

#ifdef __cplusplus
extern "C"{
#endif 

#include <stdbool.h>

///////////////////////////////
// types
///////////////////////////////

typedef struct session_data * sq_session_t;
typedef struct sequence_data * sq_sequence_t;
typedef struct trigger_data sq_trigger_t;
typedef struct inport_data * sq_inport_t;
typedef struct outport_data * sq_outport_t;

// TODO: refactor
enum inport_type {INPORT_NONE, INPORT_TRANSPOSE, INPORT_PLAYHEAD, INPORT_CLOCKDIVIDE,
                    INPORT_DIRECTION, INPORT_MUTE, INPORT_FIRST, INPORT_LAST};

///////////////////////////////
// methods
///////////////////////////////

sq_session_t sq_session_new(const char*);
int sq_session_register_outport(sq_session_t, sq_outport_t);
int sq_session_register_inport(sq_session_t, sq_inport_t);
void sq_session_start(sq_session_t);
void sq_session_stop(sq_session_t);
sq_sequence_t sq_session_get_sequence_from_name(sq_session_t, const char*);
sq_inport_t sq_session_get_inport_from_name(sq_session_t, const char*);
sq_outport_t sq_session_get_outport_from_name(sq_session_t, const char*);
void sq_session_set_bpm(sq_session_t, float);
void sq_session_add_sequence(sq_session_t, sq_sequence_t);
void sq_session_rm_sequence(sq_session_t, sq_sequence_t);
char *sq_session_get_name(sq_session_t);
float sq_session_get_bpm(sq_session_t);
void sq_session_save(sq_session_t, const char*);
sq_session_t sq_session_load(const char*);
void sq_session_teardown(sq_session_t);

sq_sequence_t sq_sequence_new(int);
void sq_sequence_delete(sq_sequence_t seq);
void sq_sequence_set_name(sq_sequence_t, const char*);
void sq_sequence_set_outport(sq_sequence_t, sq_outport_t);
void sq_sequence_set_trig(sq_sequence_t, int, sq_trigger_t*);
void sq_sequence_clear_trig(sq_sequence_t, int);
void sq_sequence_set_transpose(sq_sequence_t, int);
void sq_sequence_set_playhead(sq_sequence_t, int);
void sq_sequence_set_first(sq_sequence_t, int);
void sq_sequence_set_last(sq_sequence_t, int);
void sq_sequence_set_clockdivide(sq_sequence_t, int);
void sq_sequence_set_mute(sq_sequence_t, bool);
void sq_sequence_pprint(sq_sequence_t);
int sq_sequence_get_nsteps(sq_sequence_t);
bool sq_sequence_get_mute(sq_sequence_t);
int sq_sequence_get_transpose(sq_sequence_t);
int sq_sequence_get_clockdivide(sq_sequence_t);
int sq_sequence_get_first(sq_sequence_t);
int sq_sequence_get_last(sq_sequence_t);
void sq_sequence_set_notifications(sq_sequence_t, bool);
bool sq_sequence_read_new_playhead(sq_sequence_t, int*);
bool sq_sequence_read_new_first(sq_sequence_t, int*);
bool sq_sequence_read_new_last(sq_sequence_t, int*);
bool sq_sequence_read_new_transpose(sq_sequence_t, int*);
bool sq_sequence_read_new_clockdivide(sq_sequence_t, int*);
bool sq_sequence_read_new_mute(sq_sequence_t, bool*);

void sq_trigger_init(sq_trigger_t*);
void sq_trigger_set_null(sq_trigger_t*);
void sq_trigger_set_note(sq_trigger_t*, int, int, float);
void sq_trigger_set_cc(sq_trigger_t*, int, int);
void sq_trigger_set_probability(sq_trigger_t*, float);
void sq_trigger_set_microtime(sq_trigger_t*, float);
void sq_trigger_set_channel(sq_trigger_t*, int);

sq_inport_t sq_inport_new(const char*);
void sq_inport_delete(sq_inport_t);
void sq_inport_set_name(sq_inport_t, const char*);
void sq_inport_set_type(sq_inport_t, enum inport_type);
void sq_inport_add_sequence(sq_inport_t, sq_sequence_t);

sq_outport_t sq_outport_new(const char*);
void sq_outport_delete(sq_outport_t);
void sq_outport_set_name(sq_outport_t, const char*);
char *sq_outport_get_name(sq_outport_t);

///////////////////////////////
// old way
///////////////////////////////


// #include "sequoia/session.h"
// #include "sequoia/sequence.h"
// #include "sequoia/trigger.h"
// #include "sequoia/outport.h"
// #include "sequoia/inport.h"

///////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
