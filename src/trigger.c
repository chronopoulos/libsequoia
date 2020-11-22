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
#include "sequoia/trigger.h"

#include <string.h>

// INTERFACE CODE

sq_trigger_t sq_trigger_new(void) {

    sq_trigger_t trig;

    trig = malloc(sizeof(struct trigger_data));

    trigger_init(trig);

    trig->type = TRIG_NULL;
    trig->channel = 1;
    trig->microtime = 0.;

    trig->note_value = 60;
    trig->note_velocity = 100;
    trig->note_length = 0.5;

    trig->cc_number = 0;
    trig->cc_value = 0;

    trig->probability = 1.;

    return trig;

}

void sq_trigger_delete(sq_trigger_t trig) {

    free(trig);

}

void sq_trigger_copy(sq_trigger_t dest, sq_trigger_t src) {

    memcpy(dest, src, sizeof(struct trigger_data));

}

void sq_trigger_set_type(sq_trigger_t trig, enum trig_type type) {

    trig->type = type;

}

void sq_trigger_set_note_value(sq_trigger_t trig, int value) {

    trig->note_value = value;

}

void sq_trigger_set_note_velocity(sq_trigger_t trig, int velocity) {

    trig->note_velocity = velocity;

}

void sq_trigger_set_note_length(sq_trigger_t trig, float length) {

    if (trig->note_length > TRIG_MAX_LENGTH) trig->note_length = TRIG_MAX_LENGTH;
    trig->note_length = length;

}

void sq_trigger_set_cc_number(sq_trigger_t trig, int cc_number) {

    trig->cc_number = cc_number;

}

void sq_trigger_set_cc_value(sq_trigger_t trig, int cc_value) {

    trig->cc_value= cc_value;

}

void sq_trigger_set_probability(sq_trigger_t trig, float probability) {

    if (probability < 0.) {
        probability = 0.;
    }

    if (probability > 1.) {
        probability = 1.;
    }

    trig->probability = probability;

}

void sq_trigger_set_microtime(sq_trigger_t trig, float microtime) {

    if (microtime < -0.5) {
        microtime = -0.5;
    }

    if (microtime >= 0.5) {
        microtime = 0.4999;  // this is safe for tps < 10000 ?
    }

    trig->microtime = microtime;

}

void sq_trigger_set_channel(sq_trigger_t trig, int channel) {

  if (channel < 1) {
    channel = 1;
  }

  if (channel > 16) {
    channel = 16;
  }

  trig->channel = channel;

}

enum trig_type  sq_trigger_get_type(sq_trigger_t trig) {

    return trig->type;

}

int sq_trigger_get_note_value(sq_trigger_t trig) {

    return trig->note_value;

}

int sq_trigger_get_note_velocity(sq_trigger_t trig) {

    return trig->note_velocity;

}

float sq_trigger_get_note_length(sq_trigger_t trig) {

    return trig->note_length;

}

int sq_trigger_get_cc_number(sq_trigger_t trig) {

    return trig->cc_number;

}

int sq_trigger_get_cc_value(sq_trigger_t trig) {

    return trig->cc_value;

}

float sq_trigger_get_probability(sq_trigger_t trig) {

    return trig->probability;

}

float sq_trigger_get_microtime(sq_trigger_t trig) {

    return trig->microtime;

}

int sq_trigger_get_channel(sq_trigger_t trig) {

    return trig->channel;

}

// PUBLIC CODE

void trigger_init(sq_trigger_t trig) {

    trig->type = TRIG_NULL;
    trig->channel = 1;
    trig->microtime = 0.;

    trig->note_value = 60;
    trig->note_velocity = 100;
    trig->note_length = 0.5;

    trig->cc_number = 0;
    trig->cc_value = 0;

    trig->probability = 1.;

}

json_object *trigger_get_json(sq_trigger_t trig) {

    json_object *jo_trigger = json_object_new_object();

    json_object_object_add(jo_trigger, "type",
                            json_object_new_int(trig->type));

    json_object_object_add(jo_trigger, "channel",
                            json_object_new_int(trig->channel));

    json_object_object_add(jo_trigger, "microtime",
                            json_object_new_double(trig->microtime));

    json_object_object_add(jo_trigger, "note",
                            json_object_new_int(trig->note_value));

    json_object_object_add(jo_trigger, "velocity",
                            json_object_new_int(trig->note_velocity));

    json_object_object_add(jo_trigger, "length",
                            json_object_new_double(trig->note_length));

    json_object_object_add(jo_trigger, "cc_number",
                            json_object_new_int(trig->cc_number));

    json_object_object_add(jo_trigger, "cc_value",
                            json_object_new_int(trig->cc_value));

    json_object_object_add(jo_trigger, "probability",
                            json_object_new_double(trig->probability));

    return jo_trigger;

}

sq_trigger_t trigger_malloc_from_json(json_object *jo_trig) {

    struct json_object *jo_tmp;
    int type, channel, note_value, note_velocity, cc_number, cc_value;
    float microtime, note_length, probability;
    sq_trigger_t trig;

    // first extract the attributes

    json_object_object_get_ex(jo_trig, "type", &jo_tmp);
    type = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_trig, "channel", &jo_tmp);
    channel = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_trig, "microtime", &jo_tmp);
    microtime = json_object_get_double(jo_tmp);

    json_object_object_get_ex(jo_trig, "note", &jo_tmp);
    note_value = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_trig, "velocity", &jo_tmp);
    note_velocity = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_trig, "length", &jo_tmp);
    note_length = json_object_get_double(jo_tmp);

    json_object_object_get_ex(jo_trig, "cc_number", &jo_tmp);
    cc_number = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_trig, "cc_value", &jo_tmp);
    cc_value = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_trig, "probability", &jo_tmp);
    probability = json_object_get_double(jo_tmp);

    // then set the trig

    trig = sq_trigger_new();
    switch (type) {
        case TRIG_NULL:
            sq_trigger_set_type(trig, TRIG_NULL);
            break;
        case TRIG_NOTE:
            sq_trigger_set_type(trig, TRIG_NOTE);
            sq_trigger_set_note_value(trig, note_value);
            sq_trigger_set_note_velocity(trig, note_velocity);
            sq_trigger_set_note_length(trig, note_length);
            break;
        case TRIG_CC:
            sq_trigger_set_type(trig, TRIG_CC);
            sq_trigger_set_cc_number(trig, cc_number);
            sq_trigger_set_cc_value(trig, cc_value);
            break;
    }
    sq_trigger_set_channel(trig, channel);
    sq_trigger_set_microtime(trig, microtime);
    sq_trigger_set_probability(trig, probability);

    return trig;

}

