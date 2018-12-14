#ifndef TRIGGER_H
#define TRIGGER_H

enum trig_type {TRIG_NULL, TRIG_NOTE, TRIG_CC};

struct zig_trigger_data {

    enum trig_type type;
    int channel;            // [1, 16]
    float microtime;        // [-0.5, 0.5) in units of step

    int note;               // [0, 127]
    int velocity;           // [0, 127]
    float length;           // [0, nsteps) in units of step

    int cc_number;          // [0, 119]
    int cc_value;           // [0, 127]

};

void zig_trigger_init(struct zig_trigger_data*);
void zig_trigger_set_microtime(struct zig_trigger_data*, float);
void zig_trigger_set_note(struct zig_trigger_data*, int, int, float);

#endif
