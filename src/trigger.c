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

    trig->probability = 1.;

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

void sq_trigger_set_probability(sq_trigger_t *trig, float probability) {

    if (probability < 0.) {
        probability = 0.;
    }

    if (probability > 1.) {
        probability = 1.;
    }

    trig->probability = probability;

}

void sq_trigger_set_microtime(sq_trigger_t *trig, float microtime) {

    if (microtime < -0.5) {
        microtime = -0.5;
    }

    if (microtime >= 0.5) {
        microtime = 0.4999;  // this is safe for tps < 10000 ?
    }

    trig->microtime = microtime;

}

void sq_trigger_set_channel(sq_trigger_t *trig, int channel) {

  if (channel < 1) {
    channel = 1;
  }

  if (channel > 16) {
    channel = 16;
  }

  trig->channel = channel;

}

json_object *sq_trigger_get_json(sq_trigger_t *trig) {

    json_object *jo_trigger = json_object_new_object();

    json_object_object_add(jo_trigger, "type",
                            json_object_new_int(trig->type));

    json_object_object_add(jo_trigger, "channel",
                            json_object_new_int(trig->channel));

    json_object_object_add(jo_trigger, "microtime",
                            json_object_new_double(trig->microtime));

    json_object_object_add(jo_trigger, "note",
                            json_object_new_int(trig->note));

    json_object_object_add(jo_trigger, "velocity",
                            json_object_new_int(trig->velocity));

    json_object_object_add(jo_trigger, "length",
                            json_object_new_double(trig->length));

    json_object_object_add(jo_trigger, "cc_number",
                            json_object_new_int(trig->cc_number));

    json_object_object_add(jo_trigger, "cc_value",
                            json_object_new_int(trig->cc_value));

    json_object_object_add(jo_trigger, "probability",
                            json_object_new_double(trig->probability));

    return jo_trigger;

}

