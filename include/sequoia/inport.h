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

#ifndef inport_H
#define inport_H

#include <jack/midiport.h>

#include "sequence.h"

#define INPORT_MAX_NAME_LEN 255
#define INPORT_MAX_NSEQ 16

struct inport_data {

    enum inport_type type;

    char name[INPORT_MAX_NAME_LEN + 1];

    jack_client_t *jack_client;
    jack_port_t *jack_port;
    void *buf;

    sq_sequence_t seqs[INPORT_MAX_NSEQ];
    int nseqs;

};

void inport_process(sq_inport_t, jack_nframes_t);
json_object *inport_get_json(sq_inport_t);
sq_inport_t inport_malloc_from_json(json_object*);


#endif
