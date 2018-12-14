#include "ziggurat.h"

void zig_trigger_init(struct zig_trigger_data *trig) {

    trig->type = TRIG_NULL;
    trig->channel = 1;
    trig->microtime = 0.;

    trig->note = 60;
    trig->velocity = 100;
    trig->length = 0.5;

    trig->cc_number = 0;
    trig->cc_value = 0;

}

void zig_trigger_set_microtime(struct zig_trigger_data *trig, float microtime) {

    trig->microtime = microtime;

}

void zig_trigger_set_note(struct zig_trigger_data *trig, int note, int velocity, float length) {

    trig->type = TRIG_NOTE;
    trig->note = note;
    trig->velocity = velocity;
    trig->length = length;

}
