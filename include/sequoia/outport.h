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

#ifndef OUTPORT_H
#define OUTPORT_H

#include <jack/midiport.h>
#include <json-c/json.h> 

#define OUTPORT_MAX_NAME_LEN 255

struct outport_data {

    char name[OUTPORT_MAX_NAME_LEN + 1];

    jack_client_t *jack_client;
    jack_port_t *jack_port;
    void *buf;

};

json_object *outport_get_json(sq_outport_t);
sq_outport_t outport_malloc_from_json(json_object*);

#endif
