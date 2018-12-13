#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <jack/midiport.h>

#include "ziggurat.h"

inline jack_nframes_t _min_nframes(jack_nframes_t a, jack_nframes_t b ) {

    return a < b ? a : b;

}

void zig_sequence_init(struct zig_sequence_data *seq, int nsteps, int tps, int fpt) {

    seq->nsteps = nsteps;
    seq->tps = tps;
    seq->fpt = fpt;

    seq->nticks = seq->nsteps * seq->tps;
    seq->ticks = (int*) malloc(seq->nticks * sizeof(int));
    for(int i=0; i<seq->nticks; i++) {
        seq->ticks[i] = 0;
    }
    
    // paged timekeeping: when frame reaches fpt,
    //  it rolls over and increments tick
    seq->tick = 0;
    seq->frame = 0;

}

void zig_sequence_set_step(struct zig_sequence_data *seq, int stepIndex, int note) {

    seq->ticks[stepIndex*seq->tps] = note;

}

void zig_sequence_process(struct zig_sequence_data *seq, jack_nframes_t nframes,
                            void *port_buf) {

    char midi_msg[3];
    unsigned char *midi_msg_write_ptr;
    int note;
    jack_nframes_t nframes_left, frame_inc;

    nframes_left = nframes;

    while(nframes_left) {

        // if we're on a tick boundary, output the event
        if (seq->frame == 0) {

            if ((note = seq->ticks[seq->tick])) { // if note != 0

                midi_msg[0] = 144; // chan 1 note on
                midi_msg[1] = note;
                midi_msg[2] = 100; // velocity

                midi_msg_write_ptr = jack_midi_event_reserve(port_buf, nframes - nframes_left, 3);
                memcpy(midi_msg_write_ptr, midi_msg, 3);

            }

        }

        // increment to the next tick
        frame_inc = _min_nframes(seq->fpt - seq->frame, nframes_left);
        seq->frame += frame_inc;
        nframes_left -= frame_inc;
        if (seq->frame == seq->fpt) {
            seq->frame = 0;
            if (++(seq->tick) == seq->nticks) {
                seq->tick = 0;
            }
        }

    }

}
