#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "genseq.h"

// <helper>

inline jack_nframes_t _min_nframes(jack_nframes_t a, jack_nframes_t b ) {

    return a < b ? a : b;

}

void _get_tick_indices_note(struct gs_sequence_data *seq, int step_index, struct gs_trigger_data *trig,
                                int *tick_index_on, int *tick_index_off) {

    int index_on, index_off;

    index_on = (step_index + trig->microtime) * seq->tps;
    if (index_on < 0) index_on = 0;
    if (index_on >= seq->nticks) index_on = seq->nticks - 1;

    index_off = index_on + trig->length*seq->tps;
    if (index_off < 0) index_off = 0;
    if (index_off >= seq->nticks) index_off = seq->nticks - 1;

    *tick_index_on = index_on;
    *tick_index_off = index_off;

}

// </helper>

void gs_sequence_init(struct gs_sequence_data *seq, int nsteps, int tps, int fpt) {

    seq->nsteps = nsteps;
    seq->tps = tps;
    seq->fpt = fpt;

    seq->name[0] = '\0';
    seq->transpose = 0;

    seq->trigs = malloc(seq->nsteps * sizeof(struct gs_trigger_data));
    for (int i=0; i<seq->nsteps; i++) {
        gs_trigger_init(seq->trigs + i);
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

void gs_sequence_set_name(struct gs_sequence_data *seq, const char *name) {

    strcpy(seq->name, name);

}

void gs_sequence_set_raw_tick(struct gs_sequence_data *seq, int tick_index, midi_packet *pkt) {

    memcpy(seq->ticks + tick_index, pkt, sizeof(midi_packet));

}

void gs_sequence_set_trig(struct gs_sequence_data *seq, int step_index, struct gs_trigger_data *trig) {

    memcpy(seq->trigs + step_index, trig, sizeof(struct gs_trigger_data));

    int tick_index_on, tick_index_off;
    midi_packet pkt_on, pkt_off;
    if (trig->type == TRIG_NOTE) {

        _get_tick_indices_note(seq, step_index, trig, &tick_index_on, &tick_index_off);

        pkt_on[0] = 143 + trig->channel; // note on
        pkt_on[1] = trig->note;
        pkt_on[2] = trig->velocity;
        gs_sequence_set_raw_tick(seq, tick_index_on, &pkt_on);

        pkt_off[0] = 127 + trig->channel; // note off
        pkt_off[1] = trig->note;
        pkt_off[2] = trig->velocity; // what should we put here?
        gs_sequence_set_raw_tick(seq, tick_index_off, &pkt_off);

    }

}

void gs_sequence_clear_trig(struct gs_sequence_data *seq, int step_index) {

    int tick_index_on, tick_index_off;
    midi_packet empty_packet = {0, 0, 0};
    _get_tick_indices_note(seq, step_index, seq->trigs + step_index, &tick_index_on, &tick_index_off);

    gs_sequence_set_raw_tick(seq, tick_index_on, &empty_packet);
    gs_sequence_set_raw_tick(seq, tick_index_off, &empty_packet);

}

void gs_sequence_process(struct gs_sequence_data *seq, jack_nframes_t nframes,
                            void *port_buf) {

    unsigned char *midi_msg_write_ptr;
    jack_nframes_t nframes_left, frame_inc;

    nframes_left = nframes;

    while(nframes_left) {

        // if we're on a tick boundary, and the packet is non-empty.. output the event
        if (seq->frame == 0) {

            if (seq->ticks[seq->tick][0]) { // if status byte != 0

                midi_msg_write_ptr = jack_midi_event_reserve(port_buf, nframes - nframes_left, 3);
                memcpy(midi_msg_write_ptr, seq->ticks[seq->tick], 3);
                midi_msg_write_ptr[1] += seq->transpose;

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
