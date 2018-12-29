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

#include "sequoia.h"

void sq_trigger_init(sq_trigger_t *trig) {

    trig->type = TRIG_NULL;
    trig->channel = 1;
    trig->microtime = 0.;

    trig->note = 60;
    trig->velocity = 100;
    trig->length = 0.5;

    trig->cc_number = 0;
    trig->cc_value = 0;

}

void sq_trigger_set_null(sq_trigger_t *trig) {

    trig->type = TRIG_NULL;

}
void sq_trigger_set_note(sq_trigger_t *trig, int note, int velocity, float length) {

    trig->type = TRIG_NOTE;
    trig->note = note;
    trig->velocity = velocity;
    trig->length = length;

}

void sq_trigger_set_cc(sq_trigger_t *trig, int cc_number, int cc_value) {

    trig->type = TRIG_CC;
    trig->cc_number = cc_number;
    trig->cc_value= cc_value;

}
