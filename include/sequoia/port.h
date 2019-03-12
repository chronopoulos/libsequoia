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

#ifndef PORT_H
#define PORT_H

#include <jack/midiport.h>

#define MAX_PORT_NAME_LEN 255

enum port_type {PORT_IN, PORT_OUT};

typedef struct {

    enum port_type type;
    char name[MAX_PORT_NAME_LEN + 1];
    jack_port_t *jack_port;
    void *buf;

} sq_port_t;

void sq_port_init(sq_port_t*, enum port_type, const char*);
int sq_port_get_type(sq_port_t*);
char *sq_port_get_name(sq_port_t*);

#endif
