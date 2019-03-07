/*

    Copyright 2018, Chris Chronopoulos

    This file is part of libsequoia.

    libsequoia is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libsequoia is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libsequoia.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "sequoia.h"

// <helper>

int _get_tick_index_trig(sq_sequence_t *seq, int step_index, sq_trigger_t *trig) {

    // TODO: trig = seq->trigs + step_index ?

    int tick_index = (step_index + trig->microtime) * seq->tps;
    if (tick_index < 0) tick_index = 0;
    if (tick_index >= seq->nticks) tick_index = seq->nticks - 1;

    return tick_index;

}

void _sequence_ringbuffer_write(sq_sequence_t *seq, _sequence_ctrl_msg_t *msg) {

    int avail = jack_ringbuffer_write_space(seq->rb);
    if (avail < sizeof(_sequence_ctrl_msg_t)) {
        fprintf(stderr, "sequence ringbuffer: overflow\n");
        return;
    }

    jack_ringbuffer_write(seq->rb, (const char*) msg, sizeof(_sequence_ctrl_msg_t));

}

void _sequence_serve_ctrl_msgs(sq_sequence_t *seq) {

    int avail = jack_ringbuffer_read_space(seq->rb);
    _sequence_ctrl_msg_t msg;
    while(avail >= sizeof(_sequence_ctrl_msg_t)) {

        jack_ringbuffer_read(seq->rb, (char*) &msg, sizeof(_sequence_ctrl_msg_t));

        if (msg.param == SEQUENCE_SET_TRIG) {
            _sequence_set_trig_now(seq, msg.vi, msg.vp);
        } else if (msg.param == SEQUENCE_CLEAR_TRIG) {
            _sequence_clear_trig_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_TRANSPOSE) {
            _sequence_set_transpose_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_PH) {
            _sequence_set_playhead_now(seq, msg.vi);
        }

        avail -= sizeof(_sequence_ctrl_msg_t);

    }

}

// </helper>

void sq_sequence_init(sq_sequence_t *seq, int nsteps, int tps) {

    seq->nsteps = nsteps;
    seq->tps = tps;
    seq->name[0] = '\0';
    seq->transpose = 0;

    seq->div = 1;
    seq->idiv = 0;

    seq->trigs = malloc(seq->nsteps * sizeof(sq_trigger_t));
    for (int i=0; i<seq->nsteps; i++) {
        sq_trigger_init(seq->trigs + i);
    }

    seq->nticks = seq->nsteps * seq->tps;

    seq->microgrid = malloc(seq->nticks * sizeof(sq_trigger_t*));
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
    seq->rb = jack_ringbuffer_create(RINGBUFFER_LENGTH * sizeof(_sequence_ctrl_msg_t));
    int err = jack_ringbuffer_mlock(seq->rb);
    if (err) {
        fprintf(stderr, "failed to lock ringbuffer\n");
        exit(1);
    }

    seq->is_playing = false;

    seq->outport = NULL;
    seq->outport_buf = NULL;

}

void _sequence_prepare_outport(sq_sequence_t *seq, jack_nframes_t nframes) {

    /* this should be called once per sequence at the start of a processing callback */

    seq->outport_buf = jack_port_get_buffer(seq->outport, nframes);
    jack_midi_clear_buffer(seq->outport_buf);

}

void _sequence_tick(sq_sequence_t *seq, jack_nframes_t idx) {

    /* this should called once per sequence, every time the session frame
        counter crosses a tick boundary */

    unsigned char *midi_msg_write_ptr;
    float dice;
    sq_trigger_t *trig;
    int widx_off;

    // serve any control messages in the ringbuffer
    _sequence_serve_ctrl_msgs(seq);

    if (seq->idiv == 0) { // clock divide

        // handle any triggers from the microgrid
        if ((trig = seq->microgrid[seq->ph])) { // if non-NULL

            dice = ((float) random()) / RAND_MAX;
            if (dice <= trig->probability) {

                midi_msg_write_ptr = jack_midi_event_reserve(seq->outport_buf, idx, 3);
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

        }

        // increment ph
        if (++(seq->ph) == seq->nticks) {
            seq->ph = 0;
        }

    }

    if (++seq->idiv >= seq->div) seq->idiv = 0; // clock divide

    // handle any events in buf_off
    if (seq->buf_off[seq->ridx_off][0]) {  // if status byte != 0
        midi_msg_write_ptr = jack_midi_event_reserve(seq->outport_buf, idx, 3);
        memcpy(midi_msg_write_ptr, seq->buf_off[seq->ridx_off], 3);
        seq->buf_off[seq->ridx_off][0] = 0;  // clear this event
    }

    // increment ridx_off
    if (++(seq->ridx_off) == seq->nticks) {
        seq->ridx_off = 0;
    }

}

void sq_sequence_set_name(sq_sequence_t *seq, const char *name) {

    // this parameter is safe to touch directly (for now)
    strcpy(seq->name, name);

}

void sq_sequence_set_outport(sq_sequence_t *seq, jack_port_t *port) {

    seq->outport = port;

}

void sq_sequence_set_trig(sq_sequence_t *seq, int step_index, sq_trigger_t *trig) {

    if ( (step_index < 0) || (step_index >= seq->nsteps) ) {
        fprintf(stderr, "step index %d out of range\n", step_index);
        return;
    }

    if (seq->is_playing) {

        _sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_SET_TRIG;
        msg.vi = step_index;
        msg.vp = trig;

        _sequence_ringbuffer_write(seq, &msg);

    } else {

        _sequence_set_trig_now(seq, step_index, trig);

    }

}

void _sequence_set_trig_now(sq_sequence_t *seq, int step_index, sq_trigger_t *trig) {

    /* this function should only be called from within the audio callback */

    memcpy(seq->trigs + step_index, trig, sizeof(sq_trigger_t));
    int tick_index;
    if (trig->type == TRIG_NOTE) {
        tick_index = _get_tick_index_trig(seq, step_index, trig);
        seq->microgrid[tick_index] = seq->trigs + step_index;
    }

}


void sq_sequence_clear_trig(sq_sequence_t *seq, int step_index) {

    if ( (step_index < 0) || (step_index >= seq->nsteps) ) {
        fprintf(stderr, "step index %d out of range\n", step_index);
        return;
    }

    if (seq->is_playing) {

        _sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_SET_TRIG;
        msg.vi = step_index;

        _sequence_ringbuffer_write(seq, &msg);

    } else {

        _sequence_clear_trig_now(seq, step_index);

    }

}

void _sequence_clear_trig_now(sq_sequence_t *seq, int step_index) {

    /* this function should only be called from within the audio callback */

    sq_trigger_t *trig = seq->trigs + step_index;
    seq->microgrid[_get_tick_index_trig(seq, step_index, trig)] = NULL;
    trig->type = TRIG_NULL;

}

void sq_sequence_set_transpose(sq_sequence_t *seq, int transpose) {

    if (seq->is_playing) {

        _sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_TRANSPOSE;
        msg.vi = transpose;

        _sequence_ringbuffer_write(seq, &msg);

    } else {

        _sequence_set_transpose_now(seq, transpose);

    }

}

void _sequence_set_transpose_now(sq_sequence_t *seq, int transpose) {

    /* this function should only be called from within the audio callback */

    seq->transpose = transpose;

}

void sq_sequence_set_playhead(sq_sequence_t *seq, int ph) {

    if ( (ph < 0) || (ph >= seq->nticks) ) {
        fprintf(stderr, "playhead value out of range: %d\n", ph);
        return;
    }

    if (seq->is_playing) {

        _sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_PH;
        msg.vi = ph;

        _sequence_ringbuffer_write(seq, &msg);

    } else {

        _sequence_set_playhead_now(seq, ph);

    }

}

void _sequence_set_playhead_now(sq_sequence_t *seq, int ph) {

    /* this function should only be called from within the audio callback */

    seq->ph = ph;

}

void sq_sequence_set_clockdivide(sq_sequence_t *seq, int div) {

    if (div < 1) {
        fprintf(stderr, "clock divide of %d is out of range (must be >= 1)\n", div);
        return;
    }

    if (seq->is_playing) {

        fprintf(stderr, "set clock divide while playing not implemented yet\n");

    } else {

        _sequence_set_clockdivide_now(seq, div);

    }

}

void _sequence_set_clockdivide_now(sq_sequence_t *seq, int div) {

    seq->div = div;
    seq->idiv = 0;

}

void sq_sequence_pprint(sq_sequence_t *seq) {

    if (seq->is_playing) {

        printf("Cannot print sequence while playing.\nTry stopping the session first.\n");

    } else {

        int charCount = 0;
        bool newLineLast = false;

        for (int step = 0; step < seq->nsteps; step++) {

            if (charCount == 0) {
                printf("|");
                charCount += 1;
            }

            if (seq->trigs[step].type == TRIG_NULL) {
                printf("....|");
                charCount += 5;
                newLineLast = false;
            } else if (seq->trigs[step].type == TRIG_NOTE) {
                printf("N%03d|", seq->trigs[step].note);
                charCount += 5;
                newLineLast = false;
            } else if (seq->trigs[step].type == TRIG_CC) {
                printf("C%03d|", seq->trigs[step].cc_value);
                charCount += 5;
                newLineLast = false;
            }

            if (charCount == 81) {
                printf("\n");
                newLineLast = true;
                charCount = 0;
            }


        }

        if (!newLineLast) printf("\n");

    }

}

// read-only getters don't need to use ringbuffers

int sq_sequence_get_nsteps(sq_sequence_t *seq) {

    return seq->nsteps;

}

