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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "sequoia.h"
#include "sequoia/inport.h"

// LOCAL DECLARATIONS

unsigned int smod(int, unsigned int);
void inport_sanitize_name(sq_inport_t, const char*);

// INTERFACE CODE

sq_inport_t sq_inport_new(const char *name) {

    sq_inport_t inport;

    inport = malloc(sizeof(struct inport_data));

    inport->type = INPORT_NONE;

    inport_sanitize_name(inport, name);

    inport->jack_client = NULL;
    inport->jack_port = NULL;
    inport->buf = NULL;

    inport->nseqs = 0;

    return inport;

}

void sq_inport_delete(sq_inport_t inport) {

    free(inport);

}

void sq_inport_set_name(sq_inport_t inport, const char *name) {

    if (inport->jack_client) { // if registered
        jack_port_rename(inport->jack_client, inport->jack_port, name);
    }

    inport_sanitize_name(inport, name); 

}

void sq_inport_set_type(sq_inport_t inport, enum inport_type type) {

    inport->type = type;

}

void sq_inport_add_sequence(sq_inport_t inport, sq_sequence_t seq) {

    if (inport->nseqs >= INPORT_MAX_NSEQ) {
        fprintf(stderr, "max number of sequences per inport reached: %d\n", INPORT_MAX_NSEQ);
        return;
    }

    inport->seqs[inport->nseqs++] = seq;

}

// PUBLIC CODE

void inport_process(sq_inport_t inport, jack_nframes_t nframes) {

    int iarg;
    bool barg;

    inport->buf = jack_port_get_buffer(inport->jack_port, nframes);
    jack_nframes_t count = jack_midi_get_event_count(inport->buf);
    jack_midi_event_t ev;

    for (int i=0; i<count; i++) {

        jack_midi_event_get(&ev, inport->buf, i);

        // chan 1 note-on's only, for now
        if (ev.buffer[0] == 144) {

            switch (inport->type) {
                case INPORT_NONE:
                    break;
                case INPORT_TRANSPOSE:
                    // bipolar mapping centered at 60
                    for (int i=0; i<inport->nseqs; i++) {
                        sequence_set_transpose_now(inport->seqs[i], ev.buffer[1] - 60);
                    }
                    break;
                case INPORT_PLAYHEAD:
                    // distance from 60 is taken modulo the sequence length
                    for (int i=0; i<inport->nseqs; i++) {
                        iarg = smod(ev.buffer[1] - 60, inport->seqs[i]->nsteps);
                        sequence_set_playhead_now(inport->seqs[i], iarg);
                    }
                    break;
                case INPORT_CLOCKDIVIDE:
                    // absolute value from 60, plus 1 (so no clockdivide less than 1)
                    iarg = 1 + abs(ev.buffer[1] - 60);
                    for (int i=0; i<inport->nseqs; i++) {
                        sequence_set_clockdivide_now(inport->seqs[i], iarg);
                    }
                    break;
                case INPORT_MUTE:
                    // even=mute, odd=unmute
                    barg = ((ev.buffer[1] % 2) == 0);
                    for (int i=0; i<inport->nseqs; i++) {
                        sequence_set_mute_now(inport->seqs[i], barg);
                    }
                    break;
                case INPORT_DIRECTION:
                    fprintf(stderr, "INPORT_DIRECTION not yet implemented\n");
                    break;
                case INPORT_FIRST:
                    // distance from 60 is taken modulo the sequence length
                    for (int i=0; i<inport->nseqs; i++) {
                        iarg = smod(ev.buffer[1] - 60, inport->seqs[i]->nsteps);
                        sequence_set_first_now(inport->seqs[i], iarg);
                    }
                    break;
                case INPORT_LAST:
                    // distance from 60 is taken modulo the sequence length
                    for (int i=0; i<inport->nseqs; i++) {
                        iarg = smod(ev.buffer[1] - 60, inport->seqs[i]->nsteps);
                        sequence_set_last_now(inport->seqs[i], iarg);
                    }
                    break;
                default:
                    // this should never happen
                    fprintf(stderr, "inport has unknown type: %d\n", inport->type);
                    break;

            }

        }

    }

}

json_object *inport_get_json(sq_inport_t inport) {

    json_object *jo_inport = json_object_new_object();

    json_object_object_add(jo_inport, "name",
                            json_object_new_string(inport->name));

    json_object_object_add(jo_inport, "type",
                            json_object_new_int(inport->type));

    json_object *seq_array = json_object_new_array();
    for (int i=0; i<inport->nseqs; i++) {
        json_object_array_add(seq_array, json_object_new_string(inport->seqs[i]->name));
    }
    json_object_object_add(jo_inport, "sequences", seq_array);

    return jo_inport;

}

sq_inport_t inport_malloc_from_json(json_object *jo_inport) {

    sq_inport_t inport;
    const char *name;
    json_object *jo_tmp;
    enum inport_type type;
    

    json_object_object_get_ex(jo_inport, "name", &jo_tmp);
    name = json_object_get_string(jo_tmp);

    json_object_object_get_ex(jo_inport, "type", &jo_tmp);
    type = json_object_get_int(jo_tmp);

    inport = sq_inport_new(name);
    sq_inport_set_type(inport, type);

    return inport;

}

// LOCAL CODE

unsigned int smod(int a, unsigned int n) {

    // signed modulo

    return (a % n + n) % n;

}

void inport_sanitize_name(sq_inport_t inport, const char *name) {

    if (strlen(name) <= INPORT_MAX_NAME_LEN) {
        strcpy(inport->name, name);
    } else {
        strncpy(inport->name, name, SEQUENCE_MAX_NAME_LEN);
        inport->name[INPORT_MAX_NAME_LEN] = '\0';
    }

}

