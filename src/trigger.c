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
