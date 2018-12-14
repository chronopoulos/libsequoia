#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "ziggurat.h"

inline jack_nframes_t _min_nframes(jack_nframes_t a, jack_nframes_t b ) {

    return a < b ? a : b;

}

void zig_sequence_init(struct zig_sequence_data *seq, int nsteps, int tps, int fpt) {

    seq->nsteps = nsteps;
    seq->tps = tps;
    seq->fpt = fpt;

    seq->trigs = malloc(seq->nsteps * sizeof(struct zig_trigger_data));
    for (int i=0; i<seq->nsteps; i++) {
        zig_trigger_init(seq->trigs + i);
    }

    seq->nticks = seq->nsteps * seq->tps;

    seq->ticks = malloc(seq->nticks * sizeof(midi_packet));
    for(int i=0; i<seq->nticks; i++) {
        seq->ticks[i][0] = 0;
    }

    // paged timekeeping: when frame reaches fpt,
    //  it rolls over and increments tick
    seq->tick = 0;
    seq->frame = 0;

}

void zig_sequence_set_trig(struct zig_sequence_data *seq, int stepIndex, struct zig_trigger_data *trig) {

    memcpy(seq->trigs + stepIndex, trig, sizeof(struct zig_trigger_data));

    int tickIndex_on, tickIndex_off;
    midi_packet pkt_on, pkt_off;
    if (trig->type == TRIG_NOTE) {

        pkt_on[0] = 143 + trig->channel; // note on
        pkt_on[1] = trig->note;
        pkt_on[2] = trig->velocity;
        tickIndex_on = (stepIndex + trig->microtime)*seq->tps;
        if (tickIndex_on < 0) tickIndex_on = 0;
        if (tickIndex_on >= seq->nticks) tickIndex_on = seq->nticks - 1;
        zig_sequence_set_raw_tick(seq, tickIndex_on, &pkt_on);

        pkt_off[0] = 127 + trig->channel; // note off
        pkt_off[1] = trig->note;
        pkt_off[2] = trig->velocity; // what should we put here?
        tickIndex_off = tickIndex_on + trig->length*seq->tps;
        if (tickIndex_off < 0) tickIndex_off = 0;
        if (tickIndex_off >= seq->nticks) tickIndex_off = seq->nticks - 1;
        zig_sequence_set_raw_tick(seq, tickIndex_off, &pkt_off);

    }

}

void zig_sequence_set_raw_tick(struct zig_sequence_data *seq, int tickIndex, midi_packet *pkt) {

    memcpy(seq->ticks + tickIndex, pkt, sizeof(midi_packet));

}

void zig_sequence_process(struct zig_sequence_data *seq, jack_nframes_t nframes,
                            void *port_buf) {

    unsigned char *midi_msg_write_ptr;
    jack_nframes_t nframes_left, frame_inc;

    nframes_left = nframes;

    while(nframes_left) {

        // if we're on a tick boundary, output the event
        if (seq->frame == 0) {

            if (seq->ticks[seq->tick][0]) { // if status byte != 0

                midi_msg_write_ptr = jack_midi_event_reserve(port_buf, nframes - nframes_left, 3);
                memcpy(midi_msg_write_ptr, seq->ticks[seq->tick], 3);

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
