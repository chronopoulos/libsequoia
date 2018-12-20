#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "sequoia.h"

// <helper>

int _get_tick_index_trig(struct sq_sequence_data *seq, int step_index, struct sq_trigger_data *trig) {

    // TODO: trig = seq->trigs + step_index ?

    int tick_index = (step_index + trig->microtime) * seq->tps;
    if (tick_index < 0) tick_index = 0;
    if (tick_index >= seq->nticks) tick_index = seq->nticks - 1;

    return tick_index;

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

        if (msg.param == SEQUENCE_SET_TRIG) {
            _sequence_set_trig_now(seq, msg.vi, msg.vp);
        } else if (msg.param == SEQUENCE_CLEAR_TRIG) {
            _sequence_clear_trig_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_TRANSPOSE) {
            _sequence_set_transpose_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_PH) {
            _sequence_set_playhead_now(seq, msg.vi);
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

    seq->microgrid = malloc(seq->nticks * sizeof(struct sq_trigger_data*));
    for(int i=0; i<seq->nticks; i++) {
        seq->microgrid[i] = NULL;
    }
    seq->ph = 0;

    seq->buf_off = malloc(seq->nticks * sizeof(midi_packet));
    for(int i=0; i<seq->nticks; i++) {
        seq->buf_off[i][0] = 0;
    }
    seq->ridx_off = 0;

    // allocate and lock ringbuffer (universal ringbuffer length?)
    seq->rb = jack_ringbuffer_create(RINGBUFFER_LENGTH * sizeof(struct _sequence_ctrl_msg));
    int err = jack_ringbuffer_mlock(seq->rb);
    if (err) {
        fprintf(stderr, "failed to lock ringbuffer\n");
        exit(1);
    }

    seq->is_playing = false;

}

void sq_sequence_tick(struct sq_sequence_data *seq, void *port_buf, jack_nframes_t idx) {

    _sequence_serve_ctrl_msgs(seq);

    unsigned char *midi_msg_write_ptr;

    // handle any triggers in microgrid
    struct sq_trigger_data *trig;
    int widx_off;
    if ((trig = seq->microgrid[seq->ph])) { // if non-NULL
        midi_msg_write_ptr = jack_midi_event_reserve(port_buf, idx, 3);
        if (trig->type == TRIG_NOTE) {

            midi_msg_write_ptr[0] = 143 + trig->channel;
            midi_msg_write_ptr[1] = trig->note + seq->transpose;
            midi_msg_write_ptr[2] = trig->velocity;

            widx_off = (seq->ridx_off + (int)roundl(trig->length * seq->tps)) % seq->nticks;
            seq->buf_off[widx_off][0] = 127 + trig->channel;
            seq->buf_off[widx_off][1] = trig->note + seq->transpose;
            seq->buf_off[widx_off][2] = trig->velocity;

        }
    }

    // increment ph
    if (++(seq->ph) == seq->nticks) {
        seq->ph = 0;
    }

    // handle any events in buf_off
    if (seq->buf_off[seq->ridx_off][0]) {  // if status byte != 0
        midi_msg_write_ptr = jack_midi_event_reserve(port_buf, idx, 3);
        memcpy(midi_msg_write_ptr, seq->buf_off[seq->ridx_off], 3);
        seq->buf_off[seq->ridx_off][0] = 0;  // clear this event
    }

    // increment ridx_off
    if (++(seq->ridx_off) == seq->nticks) {
        seq->ridx_off = 0;
    }

}

void sq_sequence_set_name(struct sq_sequence_data *seq, const char *name) {

    // this parameter is safe to touch directly (for now)
    strcpy(seq->name, name);

}

void sq_sequence_set_trig(struct sq_sequence_data *seq, int step_index, struct sq_trigger_data *trig) {

    if ( (step_index < 0) || (step_index >= seq->nsteps) ) {
        fprintf(stderr, "step index %d out of range\n", step_index);
        return;
    }

    if (seq->is_playing) {

        struct _sequence_ctrl_msg msg;
        msg.param = SEQUENCE_SET_TRIG;
        msg.vi = step_index;
        msg.vp = trig;

        _sequence_ringbuffer_write(seq, &msg);

    } else {

        _sequence_set_trig_now(seq, step_index, trig);

    }

}

void _sequence_set_trig_now(struct sq_sequence_data *seq, int step_index, struct sq_trigger_data *trig) {

    /* this function should only be called from within the audio callback */

    memcpy(seq->trigs + step_index, trig, sizeof(struct sq_trigger_data));
    int tick_index;
    if (trig->type == TRIG_NOTE) {
        tick_index = _get_tick_index_trig(seq, step_index, trig);
        seq->microgrid[tick_index] = seq->trigs + step_index;
    }

}


void sq_sequence_clear_trig(struct sq_sequence_data *seq, int step_index) {

    if ( (step_index < 0) || (step_index >= seq->nsteps) ) {
        fprintf(stderr, "step index %d out of range\n", step_index);
        return;
    }

    if (seq->is_playing) {

        struct _sequence_ctrl_msg msg;
        msg.param = SEQUENCE_SET_TRIG;
        msg.vi = step_index;

        _sequence_ringbuffer_write(seq, &msg);

    } else {

        _sequence_clear_trig_now(seq, step_index);

    }

}

void _sequence_clear_trig_now(struct sq_sequence_data *seq, int step_index) {

    /* this function should only be called from within the audio callback */

    struct sq_trigger_data *trig = seq->trigs + step_index;
    seq->microgrid[_get_tick_index_trig(seq, step_index, trig)] = NULL;
    trig->type = TRIG_NULL;

}

void sq_sequence_set_transpose(struct sq_sequence_data *seq, int transpose) {

    if (seq->is_playing) {

        struct _sequence_ctrl_msg msg;
        msg.param = SEQUENCE_TRANSPOSE;
        msg.vi = transpose;

        _sequence_ringbuffer_write(seq, &msg);

    } else {

        _sequence_set_transpose_now(seq, transpose);

    }

}

void _sequence_set_transpose_now(struct sq_sequence_data *seq, int transpose) {

    /* this function should only be called from within the audio callback */

    seq->transpose = transpose;

}

void sq_sequence_set_playhead(struct sq_sequence_data *seq, int ph) {

    if ( (ph < 0) || (ph >= seq->nticks) ) {
        fprintf(stderr, "playhead value out of range: %d\n", ph);
        return;
    }

    if (seq->is_playing) {

        struct _sequence_ctrl_msg msg;
        msg.param = SEQUENCE_PH;
        msg.vi = ph;

        _sequence_ringbuffer_write(seq, &msg);

    } else {

        _sequence_set_playhead_now(seq, ph);

    }

}

void _sequence_set_playhead_now(struct sq_sequence_data *seq, int ph) {

    /* this function should only be called from within the audio callback */

    seq->ph = ph;

}
