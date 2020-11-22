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

#ifndef TRIGGER_H
#define TRIGGER_H

#include <json-c/json.h>

#define TRIG_MAX_LENGTH 16.0

struct trigger_data {

    enum trig_type type;
    int channel;            // [1, 16]
    float microtime;        // [-0.5, 0.5) in units of step
    float probability;      // [0,1]

    int note_value;         // [0, 127]
    int note_velocity;      // [0, 127]
    float note_length;      // [0, nsteps) in units of step

    int cc_number;          // [0, 119]
    int cc_value;           // [0, 127]

};

void trigger_init(sq_trigger_t);
json_object *trigger_get_json(sq_trigger_t);
sq_trigger_t trigger_malloc_from_json(json_object*);


#endif
