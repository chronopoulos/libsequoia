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

#include <stdbool.h>
#include <pthread.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>
#include <json-c/json.h>

#include "sequoia.h"
#include "trigger.h"
#include "outport.h"
#include "midiEvent.h"

// INTERFACE

#define SEQUENCE_MAX_NAME_LEN 255
#define SEQUENCE_MAX_NSTEPS 256

struct notification_data {

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

};

struct sequence_data {

    char name[SEQUENCE_MAX_NAME_LEN + 1];
    int transpose;
    struct trigger_data *trigs;
    sq_outport_t outport;
    bool is_playing;
    int nsteps;
    int step;
    jack_ringbuffer_t *rb;
    int div, idiv;
    bool mute;
    int first, last;
    struct notification_data noti;
    bool noti_enable;

};

midiEvent sequence_process(sq_sequence_t, jack_nframes_t, jack_nframes_t,
                                        jack_nframes_t, jack_nframes_t);

void sequence_step(sq_sequence_t);

json_object *sequence_get_json(sq_sequence_t);
sq_sequence_t sequence_malloc_from_json(json_object*);

void sequence_reset_now(sq_sequence_t);
void sequence_set_trig_now(sq_sequence_t, int, sq_trigger_t);
void sequence_clear_trig_now(sq_sequence_t, int);
void sequence_set_transpose_now(sq_sequence_t, int);
void sequence_set_playhead_now(sq_sequence_t, int);
void sequence_set_first_now(sq_sequence_t, int);
void sequence_set_last_now(sq_sequence_t, int);
void sequence_set_clockdivide_now(sq_sequence_t, int);
void sequence_set_mute_now(sq_sequence_t, bool);

#endif
