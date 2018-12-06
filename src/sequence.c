#include <stdlib.h>
#include <stdio.h>

#include "ziggurat.h"

void zig_sequence_init(struct zig_sequence_data *seq, int length) {

    seq->length = length;
    seq->trigs = (int*) malloc(seq->length * sizeof(int));

    for(int i=0; i<seq->length; i++) {
        seq->trigs[i] = 0;
    }

    seq->playhead = 0;

}

/* set ringbuffer for input */
void zig_sequence_set_rbi(struct zig_sequence_data *seq, jack_ringbuffer_t *rbi) {

    seq->rbi = rbi;

}

/* set ringbuffer for output */
void zig_sequence_set_rbo(struct zig_sequence_data *seq, jack_ringbuffer_t *rbo) {

    seq->rbo = rbo;

}

void zig_sequence_set_trig(struct zig_sequence_data *seq, int i, int trig) {

    if ((0 <= i) && (i < seq->length)) {
        seq->trigs[i] = trig;
    }

}

void zig_sequence_tick(struct zig_sequence_data *seq) {

    char midi_msg_buf[3];

    int note;
    if ((note = seq->trigs[seq->playhead])) { // if note != 0

        midi_msg_buf[0] = 144; // chan 1 note on
        midi_msg_buf[1] = note;
        midi_msg_buf[2] = 100; // velocity
        jack_ringbuffer_write(seq->rbo, midi_msg_buf, 3);

    }

    if (seq->playhead == (seq->length - 1)) {
        seq->playhead = 0;
    } else {
        seq->playhead++;
    }

}

