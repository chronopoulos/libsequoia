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

#include "sequoia.h"

void _outport_sanitize_name(sq_outport_t *outport, const char *name) {

    if (strlen(name) <= OUTPORT_MAX_NAME_LEN) {
        strcpy(outport->name, name);
    } else {
        strncpy(outport->name, name, OUTPORT_MAX_NAME_LEN);
        outport->name[OUTPORT_MAX_NAME_LEN] = '\0';
    }

}

void sq_outport_init(sq_outport_t *outport, const char *name) {

    _outport_sanitize_name(outport, name);

    outport->jack_client = NULL;
    outport->jack_port = NULL;
    outport->buf = NULL;

}

void sq_outport_set_name(sq_outport_t *outport, const char *name) {

    if (outport->jack_client) { // if registered
        jack_port_rename(outport->jack_client, outport->jack_port, name);
    }

    _outport_sanitize_name(outport, name); 

}

char *sq_outport_get_name(sq_outport_t *outport) {

    return outport->name;

}

json_object *sq_outport_get_json(sq_outport_t *outport) {

    json_object *jo_outport = json_object_new_object();

    json_object_object_add(jo_outport, "name",
                            json_object_new_string(sq_outport_get_name(outport)));

    return jo_outport;

}

sq_outport_t *sq_outport_malloc_from_json(json_object *jo_outport) {

    sq_outport_t *outport;
    const char *name;
    json_object *jo_tmp;

    json_object_object_get_ex(jo_outport, "name", &jo_tmp);
    name = json_object_get_string(jo_tmp);

    outport = malloc(sizeof(sq_outport_t));
    sq_outport_init(outport, name);

    return outport;

}

