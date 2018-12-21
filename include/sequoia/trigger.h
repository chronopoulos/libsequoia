#ifndef TRIGGER_H
#define TRIGGER_H

enum trig_type {TRIG_NULL, TRIG_NOTE, TRIG_CC};

typedef struct {

    enum trig_type type;
    int channel;            // [1, 16]
    float microtime;        // [-0.5, 0.5) in units of step

    int note;               // [0, 127]
    int velocity;           // [0, 127]
    float length;           // [0, nsteps) in units of step

    int cc_number;          // [0, 119]
    int cc_value;           // [0, 127]

} sq_trigger_t;

void sq_trigger_init(sq_trigger_t*);
void sq_trigger_set_null(sq_trigger_t*);
void sq_trigger_set_note(sq_trigger_t*, int, int, float);
void sq_trigger_set_cc(sq_trigger_t*, int, int);

#endif
