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
#include <stddef.h>

///////////////////////////////
// types
///////////////////////////////

typedef struct session_data * sq_session_t;
typedef struct sequence_data * sq_sequence_t;
typedef struct trigger_data * sq_trigger_t;
typedef struct inport_data * sq_inport_t;
typedef struct outport_data * sq_outport_t;

///////////////////////////////
// enums
///////////////////////////////

enum inport_type {INPORT_NONE, INPORT_TRANSPOSE, INPORT_PLAYHEAD, INPORT_CLOCKDIVIDE,
                    INPORT_DIRECTION, INPORT_MUTE, INPORT_FIRST, INPORT_LAST};

enum trig_type {TRIG_NULL, TRIG_NOTE, TRIG_CC};

///////////////////////////////
// methods
///////////////////////////////

sq_session_t    sq_session_new(const char*);
void            sq_session_delete(sq_session_t);
void            sq_session_delete_recursive(sq_session_t);
int             sq_session_register_outport(sq_session_t, sq_outport_t);
int             sq_session_register_inport(sq_session_t, sq_inport_t);
void            sq_session_add_sequence(sq_session_t, sq_sequence_t);
void            sq_session_rm_sequence(sq_session_t, sq_sequence_t);
void            sq_session_start(sq_session_t);
void            sq_session_stop(sq_session_t);
void            sq_session_set_bpm(sq_session_t, float);
const char*     sq_session_get_name(sq_session_t);
float           sq_session_get_bpm(sq_session_t);
size_t          sq_session_get_nseqs(sq_session_t);
sq_sequence_t   sq_session_get_seq(sq_session_t, size_t);
size_t          sq_session_get_ninports(sq_session_t);
sq_inport_t     sq_session_get_inport(sq_session_t, size_t);
size_t          sq_session_get_noutports(sq_session_t);
sq_outport_t    sq_session_get_outport(sq_session_t, size_t);
void            sq_session_save(sq_session_t, const char*);
sq_session_t    sq_session_load(const char*);

sq_sequence_t   sq_sequence_new(int);
void            sq_sequence_delete(sq_sequence_t);
void            sq_sequence_pprint(sq_sequence_t);
////
sq_outport_t    sq_sequence_get_outport(sq_sequence_t);
void            sq_sequence_set_outport(sq_sequence_t, sq_outport_t);
sq_trigger_t    sq_sequence_get_trig(sq_sequence_t, size_t);
void            sq_sequence_set_trig(sq_sequence_t, int, sq_trigger_t);
void            sq_sequence_clear_trig(sq_sequence_t, int);
////
const char*     sq_sequence_get_name(sq_sequence_t);
void            sq_sequence_set_name(sq_sequence_t, const char*);
int             sq_sequence_get_transpose(sq_sequence_t);
void            sq_sequence_set_transpose(sq_sequence_t, int);
int             sq_sequence_get_playhead(sq_sequence_t);
void            sq_sequence_set_playhead(sq_sequence_t, int);
int             sq_sequence_get_first(sq_sequence_t);
void            sq_sequence_set_first(sq_sequence_t, int);
int             sq_sequence_get_last(sq_sequence_t);
void            sq_sequence_set_last(sq_sequence_t, int);
int             sq_sequence_get_clockdivide(sq_sequence_t);
void            sq_sequence_set_clockdivide(sq_sequence_t, int);
bool            sq_sequence_get_mute(sq_sequence_t);
void            sq_sequence_set_mute(sq_sequence_t, bool);
int             sq_sequence_get_nsteps(sq_sequence_t);
////
void            sq_sequence_set_notifications(sq_sequence_t, bool);
bool            sq_sequence_read_new_playhead(sq_sequence_t, int*);
bool            sq_sequence_read_new_first(sq_sequence_t, int*);
bool            sq_sequence_read_new_last(sq_sequence_t, int*);
bool            sq_sequence_read_new_transpose(sq_sequence_t, int*);
bool            sq_sequence_read_new_clockdivide(sq_sequence_t, int*);
bool            sq_sequence_read_new_mute(sq_sequence_t, bool*);

sq_trigger_t    sq_trigger_new(void);
void            sq_trigger_delete(sq_trigger_t);
void            sq_trigger_copy(sq_trigger_t, sq_trigger_t);
void            sq_trigger_set_type(sq_trigger_t, enum trig_type);
void            sq_trigger_set_note_value(sq_trigger_t, int);
void            sq_trigger_set_note_velocity(sq_trigger_t, int);
void            sq_trigger_set_note_length(sq_trigger_t, float);
void            sq_trigger_set_cc_number(sq_trigger_t, int);
void            sq_trigger_set_cc_value(sq_trigger_t, int);
void            sq_trigger_set_probability(sq_trigger_t, float);
void            sq_trigger_set_microtime(sq_trigger_t, float);
void            sq_trigger_set_channel(sq_trigger_t, int);
enum trig_type  sq_trigger_get_type(sq_trigger_t);
int             sq_trigger_get_note_value(sq_trigger_t);
int             sq_trigger_get_note_velocity(sq_trigger_t);
float           sq_trigger_get_note_length(sq_trigger_t);
int             sq_trigger_get_cc_number(sq_trigger_t);
int             sq_trigger_get_cc_value(sq_trigger_t);
float           sq_trigger_get_probability(sq_trigger_t);
float           sq_trigger_get_microtime(sq_trigger_t);
int             sq_trigger_get_channel(sq_trigger_t);

sq_inport_t         sq_inport_new(const char*);
void                sq_inport_delete(sq_inport_t);
void                sq_inport_set_name(sq_inport_t, const char*);
void                sq_inport_set_type(sq_inport_t, enum inport_type);
void                sq_inport_add_sequence(sq_inport_t, sq_sequence_t);
const char*         sq_inport_get_name(sq_inport_t);
enum inport_type    sq_inport_get_type(sq_inport_t);

sq_outport_t    sq_outport_new(const char*);
void            sq_outport_delete(sq_outport_t);
void            sq_outport_set_name(sq_outport_t, const char*);
char*           sq_outport_get_name(sq_outport_t);

#ifdef __cplusplus
}
#endif

#endif
