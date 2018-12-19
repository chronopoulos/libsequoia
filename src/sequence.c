#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "sequoia.h"

// <helper>

void _get_tick_indices_note(struct sq_sequence_data *seq, int step_index, struct sq_trigger_data *trig,
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

void _sequence_ringbuffer_write(struct sq_sequence_data *seq, struct _sequence_ctrl_msg *msg) {

    int avail = jack_ringbuffer_write_space(seq->rb);
    if (avail < sizeof(struct _sequence_ctrl_msg)) {
        fprintf(stderr, "sequence ringbuffer: overflow\n");
        return;
    }

    jack_ringbuffer_write(seq->rb, (const char*) msg, sizeof(struct _sequence_ctrl_msg));

}

void _sequence_serve_ctrl_msgs(struct sq_sequence_data *seq) {

    int avail = jack_ringbuffer_read_space(seq->rb);
    struct _sequence_ctrl_msg msg;
    while(avail >= sizeof(struct _sequence_ctrl_msg)) {

        jack_ringbuffer_read(seq->rb, (char*) &msg, sizeof(struct _sequence_ctrl_msg));

        if (msg.param == SEQUENCE_TRANSPOSE) {
            seq->transpose = msg.vi;
        } else if (msg.param == SEQUENCE_TICK) {
            seq->tick = msg.vi;
        }

        avail -= sizeof(struct _sequence_ctrl_msg);

    }

}

// </helper>

void sq_sequence_init(struct sq_sequence_data *seq, int nsteps, int tps) {

    seq->nsteps = nsteps;
    seq->tps = tps;
    seq->name[0] = '\0';
    seq->transpose = 0;

    seq->trigs = malloc(seq->nsteps * sizeof(struct sq_trigger_data));
    for (int i=0; i<seq->nsteps; i++) {
        sq_trigger_init(seq->trigs + i);
    }

    seq->nticks = seq->nsteps * seq->tps;
    seq->ticks = malloc(seq->nticks * sizeof(midi_packet));
    for(int i=0; i<seq->nticks; i++) {
        seq->ticks[i][0] = 0;
    }

    seq->tick = 0;

    // allocate and lock ringbuffer (universal ringbuffer length?)
    seq->rb = jack_ringbuffer_create(RINGBUFFER_LENGTH * sizeof(struct _sequence_ctrl_msg));
    int err = jack_ringbuffer_mlock(seq->rb);
    if (err) {
        fprintf(stderr, "failed to lock ringbuffer\n");
        exit(1);
    }


}

void sq_sequence_set_name(struct sq_sequence_data *seq, const char *name) {

    strcpy(seq->name, name);

}

void sq_sequence_set_raw_tick(struct sq_sequence_data *seq, int tick_index, midi_packet *pkt) {

    memcpy(seq->ticks + tick_index, pkt, sizeof(midi_packet));

}

void sq_sequence_set_trig(struct sq_sequence_data *seq, int step_index, struct sq_trigger_data *trig) {

    memcpy(seq->trigs + step_index, trig, sizeof(struct sq_trigger_data));

    int tick_index_on, tick_index_off;
    midi_packet pkt_on, pkt_off;
    if (trig->type == TRIG_NOTE) {

        _get_tick_indices_note(seq, step_index, trig, &tick_index_on, &tick_index_off);

        pkt_on[0] = 143 + trig->channel; // note on
        pkt_on[1] = trig->note;
        pkt_on[2] = trig->velocity;
        sq_sequence_set_raw_tick(seq, tick_index_on, &pkt_on);

        pkt_off[0] = 127 + trig->channel; // note off
        pkt_off[1] = trig->note;
        pkt_off[2] = trig->velocity; // what should we put here?
        sq_sequence_set_raw_tick(seq, tick_index_off, &pkt_off);

    }

}

void sq_sequence_clear_trig(struct sq_sequence_data *seq, int step_index) {

    int tick_index_on, tick_index_off;
    midi_packet empty_packet = {0, 0, 0};
    _get_tick_indices_note(seq, step_index, seq->trigs + step_index, &tick_index_on, &tick_index_off);

    sq_sequence_set_raw_tick(seq, tick_index_on, &empty_packet);
    sq_sequence_set_raw_tick(seq, tick_index_off, &empty_packet);

}

void sq_sequence_tick(struct sq_sequence_data *seq, void *port_buf, jack_nframes_t idx) {

    _sequence_serve_ctrl_msgs(seq);

    unsigned char *midi_msg_write_ptr;

    // if the packet is non-empty, output the event
    if (seq->ticks[seq->tick][0]) { // if status byte != 0

        midi_msg_write_ptr = jack_midi_event_reserve(port_buf, idx, 3);
        memcpy(midi_msg_write_ptr, seq->ticks[seq->tick], 3);

        // apply transpose
        midi_msg_write_ptr[1] += seq->transpose;

    }

    // increment tick counter
    if (++(seq->tick) == seq->nticks) {
        seq->tick = 0;
    }

}

void sq_sequence_set_transpose(struct sq_sequence_data *seq, int transpose) {

    struct _sequence_ctrl_msg msg;
    msg.param = SEQUENCE_TRANSPOSE;
    msg.vi = transpose;

    _sequence_ringbuffer_write(seq, &msg);

}

void sq_sequence_set_tick(struct sq_sequence_data *seq, int tick) {

    if ( (tick < 0) || (tick >= seq->nticks) ) {
        fprintf(stderr, "tick value out of range: %d\n", tick);
        return;
    }

    struct _sequence_ctrl_msg msg;
    msg.param = SEQUENCE_TICK;
    msg.vi = tick;

    _sequence_ringbuffer_write(seq, &msg);

}

