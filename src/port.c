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

void sq_port_init(sq_port_t *port, enum port_type type, const char *name) {

    port->type = type;

    if (strlen(name) <= MAX_PORT_NAME_LEN) {
        strcpy(port->name, name);
    } else {
        strncpy(port->name, name, MAX_SEQ_NAME_LEN);
        port->name[MAX_SEQ_NAME_LEN] = '\0';
    }


    port->jack_port = NULL;
    port->buf = NULL;

}

int sq_port_get_type(sq_port_t *port) {

    return port->type;

}

char *sq_port_get_name(sq_port_t *port) {

    return port->name;

}
