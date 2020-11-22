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
#include "sequoia/sequence.h"
#include "sequoia/midiEvent.h"

// LOCAL DECLARATIONS

#define SEQUENCE_RB_LENGTH 16

enum sequence_param {SEQUENCE_SET_TRIG, SEQUENCE_CLEAR_TRIG, SEQUENCE_TRANSPOSE, SEQUENCE_PH,
                        SEQUENCE_DIV, SEQUENCE_MUTE, SEQUENCE_FIRST, SEQUENCE_LAST};

typedef struct {

    enum sequence_param param;

    // parameter-dependent value fields
    int vi;
    float vf;
    bool vb;
    sq_trigger_t vp;

} sequence_ctrl_msg_t;

static void sequence_ringbuffer_write(sq_sequence_t, sequence_ctrl_msg_t*);
static void sequence_serve_ctrl_msgs(sq_sequence_t);
static void notification_data_init(struct notification_data*);

// INTERFACE CODE

sq_sequence_t sq_sequence_new(int nsteps) {

    sq_sequence_t seq;

    if (nsteps > SEQUENCE_MAX_NSTEPS) {
        fprintf(stderr, "nsteps of %d exceeds max of %d\n", nsteps, SEQUENCE_MAX_NSTEPS);
        exit(1);
    }

    seq = malloc(sizeof(struct sequence_data));

    seq->nsteps = nsteps;
    seq->name[0] = '\0';
    seq->transpose = 0;

    seq->div = 1;

    // TODO switch to mallocing dynamically?
    seq->trigs = malloc(seq->nsteps * sizeof(struct trigger_data));
    for (int i=0; i<seq->nsteps; i++) {
        trigger_init(seq->trigs + i);
    }

    // allocate and lock ringbuffer (universal ringbuffer length?)
    seq->rb = jack_ringbuffer_create(SEQUENCE_RB_LENGTH * sizeof(sequence_ctrl_msg_t));
    int err = jack_ringbuffer_mlock(seq->rb);
    if (err) {
        fprintf(stderr, "failed to lock ringbuffer\n");
        exit(1);
    }

    seq->is_playing = false;

    seq->outport = NULL;

    seq->mute = false;

    seq->first = 0;
    seq->last = seq->nsteps - 1;

    notification_data_init(&seq->noti);
    seq->noti_enable = false;

    sequence_reset_now(seq);

    return seq;

}

void sq_sequence_delete(sq_sequence_t seq) {

    free(seq->trigs);
    free(seq);

}

void sq_sequence_set_name(sq_sequence_t seq, const char *name) {

    // this parameter is safe to touch directly (for now)

    if (strlen(name) <= SEQUENCE_MAX_NAME_LEN) {
        strcpy(seq->name, name);
    } else {
        strncpy(seq->name, name, SEQUENCE_MAX_NAME_LEN);
        seq->name[SEQUENCE_MAX_NAME_LEN] = '\0';
    }

}

void sq_sequence_set_outport(sq_sequence_t seq, sq_outport_t outport) {

    seq->outport = outport;

}

void sq_sequence_set_trig(sq_sequence_t seq, int step_index, sq_trigger_t trig) {

    if (seq->is_playing) {

        sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_SET_TRIG;
        msg.vi = step_index;
        msg.vp = trig;

        sequence_ringbuffer_write(seq, &msg);

    } else {

        sequence_set_trig_now(seq, step_index, trig);

    }

}

void sq_sequence_clear_trig(sq_sequence_t seq, int step_index) {

    if (seq->is_playing) {

        sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_SET_TRIG;
        msg.vi = step_index;

        sequence_ringbuffer_write(seq, &msg);

    } else {

        sequence_clear_trig_now(seq, step_index);

    }

}

void sq_sequence_set_transpose(sq_sequence_t seq, int transpose) {

    if (seq->is_playing) {

        sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_TRANSPOSE;
        msg.vi = transpose;

        sequence_ringbuffer_write(seq, &msg);

    } else {

        sequence_set_transpose_now(seq, transpose);

    }

}

void sq_sequence_set_playhead(sq_sequence_t seq, int ph) {

    if (seq->is_playing) {

        sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_PH;
        msg.vi = ph;

        sequence_ringbuffer_write(seq, &msg);

    } else {

        sequence_set_playhead_now(seq, ph);

    }

}

void sq_sequence_set_first(sq_sequence_t seq, int first) {

    if (seq->is_playing) {

        sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_FIRST;
        msg.vi = first;

        sequence_ringbuffer_write(seq, &msg);

    } else {

        sequence_set_first_now(seq, first);

    }

}

void sq_sequence_set_last(sq_sequence_t seq, int last) {

    if (seq->is_playing) {

        sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_LAST;
        msg.vi = last;

        sequence_ringbuffer_write(seq, &msg);

    } else {

        sequence_set_last_now(seq, last);

    }

}

void sq_sequence_set_clockdivide(sq_sequence_t seq, int div) {

    if (seq->is_playing) {

        sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_DIV;
        msg.vi = div;

        sequence_ringbuffer_write(seq, &msg);

    } else {

        sequence_set_clockdivide_now(seq, div);

    }

}

void sq_sequence_set_mute(sq_sequence_t seq, bool mute) {

    if (seq->is_playing) {

        sequence_ctrl_msg_t msg;
        msg.param = SEQUENCE_MUTE;
        msg.vb = mute;

        sequence_ringbuffer_write(seq, &msg);

    } else {

        sequence_set_mute_now(seq, mute);

    }

}

void sq_sequence_pprint(sq_sequence_t seq) {

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
                printf("N%03d|", seq->trigs[step].note_value);
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

void sq_sequence_set_notifications(sq_sequence_t seq, bool enable) {

    seq->noti_enable = enable;

}

bool sq_sequence_read_new_playhead(sq_sequence_t seq, int *val) {

    bool new;

    if ((new = seq->noti.playhead_new)) {
        *val = seq->noti.playhead;
        seq->noti.playhead_new = false;
    }

    return new;

}

bool sq_sequence_read_new_first(sq_sequence_t seq, int *val) {

    bool new;

    if ((new = seq->noti.first_new)) {
        *val = seq->noti.first;
        seq->noti.first_new = false;
    }

    return new;

}

bool sq_sequence_read_new_last(sq_sequence_t seq, int *val) {

    bool new;

    if ((new = seq->noti.last_new)) {
        *val = seq->noti.last;
        seq->noti.last_new = false;
    }

    return new;

}

bool sq_sequence_read_new_transpose(sq_sequence_t seq, int *val) {

    bool new;

    if ((new = seq->noti.transpose_new)) {
        *val = seq->noti.transpose;
        seq->noti.transpose_new = false;
    }

    return new;
}

bool sq_sequence_read_new_clockdivide(sq_sequence_t seq, int *val) {

    bool new;

    if ((new = seq->noti.clockdivide_new)) {
        *val = seq->noti.clockdivide;
        seq->noti.clockdivide_new = false;
    }

    return new;
}

bool sq_sequence_read_new_mute(sq_sequence_t seq, bool *val) {

    bool new;

    if ((new = seq->noti.mute_new)) {
        *val = seq->noti.mute;
        seq->noti.mute_new = false;
    }

    return new;
}

// read-only getters don't need to use ringbuffers

int sq_sequence_get_playhead(sq_sequence_t seq) {

    return seq->step;

}

const char *sq_sequence_get_name(sq_sequence_t seq) {

    return seq->name;

}

sq_outport_t sq_sequence_get_outport(sq_sequence_t seq) {

    return seq->outport;

}

int sq_sequence_get_nsteps(sq_sequence_t seq) {

    return seq->nsteps;

}

sq_trigger_t sq_sequence_get_trig(sq_sequence_t seq, size_t index) {

    return seq->trigs + index;

}

bool sq_sequence_get_mute(sq_sequence_t seq) {

    return seq->mute;

}

int sq_sequence_get_transpose(sq_sequence_t seq) {

    return seq->transpose;

}

int sq_sequence_get_clockdivide(sq_sequence_t seq) {

    return seq->div;

}

int sq_sequence_get_first(sq_sequence_t seq) {

    return seq->first;

}

int sq_sequence_get_last(sq_sequence_t seq) {

    return seq->last;

}

// PUBLIC CODE

void sequence_reset_now(sq_sequence_t seq) {

    seq->idiv = 0;
    seq->step = seq->first;

    if (seq->noti_enable) {
        seq->noti.playhead = seq->first;
        seq->noti.playhead_new = true;
    }

}

midiEvent sequence_process(sq_sequence_t seq, jack_nframes_t fps,
                        jack_nframes_t start, jack_nframes_t len, jack_nframes_t buf_offset) {

    sq_trigger_t trig;
    jack_nframes_t frame_trig;
    midiEvent mev; // tmp value

    if (start + len > fps) {   // this should never happen
        fprintf(stderr, "sequence_process() crossed step boundary: %s\n", seq->name);
        return MIDIEVENT_NULL;
    }

    // serve any control messages in the ringbuffer
    sequence_serve_ctrl_msgs(seq);

    // output JACK MIDI
    if (!seq->mute && seq->outport && !seq->idiv) {
        trig = seq->trigs + seq->step;
        if (trig->type != TRIG_NULL) {
            if (trig->probability >= ((float) random()) / RAND_MAX) {
                frame_trig = fps * (0.5 + trig->microtime);  // round down
                if ((frame_trig >= start) && (frame_trig < start + len)) {

                    mev.buf = seq->outport->buf;
                    mev.time = frame_trig - start;
                    if (trig->type == TRIG_NOTE) {
                        mev.type = MEV_TYPE_NOTEON;
                        mev.status = 143 + trig->channel;   // note on
                        mev.data1 = trig->note_value + seq->transpose;
                        mev.data2 = trig->note_velocity;
                        mev.length = trig->note_length * fps;
                    } else if (trig->type == TRIG_CC) {
                        mev.type = MEV_TYPE_CC;
                        mev.status = 175 + trig->channel;   // control change
                        mev.data1 = trig->cc_number;
                        mev.data2 = trig->cc_value;
                    }

                    return mev;

                }
            }
        }
    }

    return MIDIEVENT_NULL;

}

void sequence_step(sq_sequence_t seq) {

    seq->idiv++;

    if (seq->idiv == seq->div) {

        // increment
        if (seq->step == seq->last) {
            seq->step = seq->first;
        } else if (++(seq->step) == seq->nsteps) {
            seq->step = 0;
        }

        // and send a notification
        if (seq->noti_enable) {
            seq->noti.playhead = seq->step;
            seq->noti.playhead_new = true;
        }

        // and reset the counter
        seq->idiv = 0;

    }

}

json_object *sequence_get_json(sq_sequence_t seq) {

    json_object *jo_sequence = json_object_new_object();

    json_object_object_add(jo_sequence, "name",
                            json_object_new_string(seq->name));

    json_object_object_add(jo_sequence, "nsteps",
                            json_object_new_int(sq_sequence_get_nsteps(seq)));

    json_object_object_add(jo_sequence, "mute",
                            json_object_new_boolean(sq_sequence_get_mute(seq)));

    json_object_object_add(jo_sequence, "transpose",
                            json_object_new_int(sq_sequence_get_transpose(seq)));

    json_object_object_add(jo_sequence, "clockdivide",
                            json_object_new_int(sq_sequence_get_clockdivide(seq)));

    json_object_object_add(jo_sequence, "first",
                            json_object_new_int(sq_sequence_get_first(seq)));

    json_object_object_add(jo_sequence, "last",
                            json_object_new_int(sq_sequence_get_last(seq)));

    json_object *trigger_array = json_object_new_array();
    for (int i=0; i<seq->nsteps; i++) {
        json_object_array_add(trigger_array, trigger_get_json(seq->trigs + i));
    }
    json_object_object_add(jo_sequence, "triggers", trigger_array);

    if (seq->outport) {
        json_object_object_add(jo_sequence, "outport",
                                json_object_new_string(seq->outport->name));
    } else {
        json_object_object_add(jo_sequence, "outport", NULL);
    }

    return jo_sequence;

}

sq_sequence_t sequence_malloc_from_json(json_object *jo_seq) {

    struct json_object *jo_tmp, *jo_trig;
    const char *name;
    int nsteps, transpose, clockdivide, first, last;
    bool mute;
    sq_sequence_t seq;
    sq_trigger_t trig;

    // first extract the top-level attributes

    json_object_object_get_ex(jo_seq, "name", &jo_tmp);
    name = json_object_get_string(jo_tmp);

    json_object_object_get_ex(jo_seq, "nsteps", &jo_tmp);
    nsteps = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_seq, "mute", &jo_tmp);
    mute = json_object_get_boolean(jo_tmp);

    json_object_object_get_ex(jo_seq, "transpose", &jo_tmp);
    transpose = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_seq, "clockdivide", &jo_tmp);
    clockdivide = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_seq, "first", &jo_tmp);
    first = json_object_get_int(jo_tmp);

    json_object_object_get_ex(jo_seq, "last", &jo_tmp);
    last = json_object_get_int(jo_tmp);

    // malloc and init the sequence

    seq = sq_sequence_new(nsteps);
    sq_sequence_set_name(seq, name);
    sq_sequence_set_mute(seq, mute);
    sq_sequence_set_transpose(seq, transpose);
    sq_sequence_set_clockdivide(seq, clockdivide);
    sq_sequence_set_first(seq, first);
    sq_sequence_set_last(seq, last);

    // then set the triggers
    json_object_object_get_ex(jo_seq, "triggers", &jo_tmp);
    for (int i=0; i<nsteps; i++) {
        jo_trig = json_object_array_get_idx(jo_tmp, i);
        // trigs get copied into seqs, so we don't need to malloc them
        trig = trigger_malloc_from_json(jo_trig);
        sq_sequence_set_trig(seq, i, trig);
    }

    return seq;

}

void sequence_set_trig_now(sq_sequence_t seq, int step_index, sq_trigger_t trig) {

    if ( (step_index < 0) || (step_index >= seq->nsteps) ) {
        fprintf(stderr, "step index %d out of range\n", step_index);
        return;
    }

    memcpy(seq->trigs + step_index, trig, sizeof(struct trigger_data));

}

void sequence_clear_trig_now(sq_sequence_t seq, int step_index) {

    if ( (step_index < 0) || (step_index >= seq->nsteps) ) {
        fprintf(stderr, "step index %d out of range\n", step_index);
        return;
    }

    sq_trigger_t trig = seq->trigs + step_index;
    trig->type = TRIG_NULL;

}

void sequence_set_transpose_now(sq_sequence_t seq, int transpose) {

    seq->transpose = transpose;

    if (seq->noti_enable) {
        seq->noti.transpose = transpose;
        seq->noti.transpose_new = true;
    }

}

void sequence_set_playhead_now(sq_sequence_t seq, int ph) {

    if ( (ph < 0) || (ph >= seq->nsteps) ) {
        fprintf(stderr, "playhead value out of range: %d\n", ph);
        return;
    }

    seq->step = ph;

    if (seq->noti_enable) {
        seq->noti.playhead = ph;
        seq->noti.playhead_new = true;
    }

}

void sequence_set_first_now(sq_sequence_t seq, int first) {

    if ( (first < 0) || (first >= seq->nsteps) ) {
        fprintf(stderr, "first value out of range: %d\n", first);
        return;
    }

    seq->first = first;

    if (seq->noti_enable) {
        seq->noti.first = first;
        seq->noti.first_new = true;
    }


}

void sequence_set_last_now(sq_sequence_t seq, int last) {

    if ( (last < 0) || (last >= seq->nsteps) ) {
        fprintf(stderr, "last value out of range: %d\n", last);
        return;
    }

    seq->last = last;

    if (seq->noti_enable) {
        seq->noti.last = last;
        seq->noti.last_new = true;
    }

}

void sequence_set_clockdivide_now(sq_sequence_t seq, int div) {

    if (div < 1) {
        fprintf(stderr, "clock divide of %d is out of range (must be >= 1)\n", div);
        return;
    }

    seq->div = div;
    seq->idiv = 0;

    if (seq->noti_enable) {
        seq->noti.clockdivide = div;
        seq->noti.clockdivide_new = true;
    }

}

void sequence_set_mute_now(sq_sequence_t seq, bool mute) {

    seq->mute = mute;

    if (seq->noti_enable) {
        seq->noti.mute = mute;
        seq->noti.mute_new = true;
    }

}

// STATIC CODE

static void sequence_ringbuffer_write(sq_sequence_t seq, sequence_ctrl_msg_t *msg) {

    int avail = jack_ringbuffer_write_space(seq->rb);
    if (avail < sizeof(sequence_ctrl_msg_t)) {
        fprintf(stderr, "sequence ringbuffer: overflow\n");
        return;
    }

    jack_ringbuffer_write(seq->rb, (const char*) msg, sizeof(sequence_ctrl_msg_t));

}

static void sequence_serve_ctrl_msgs(sq_sequence_t seq) {

    int avail = jack_ringbuffer_read_space(seq->rb);
    sequence_ctrl_msg_t msg;
    while(avail >= sizeof(sequence_ctrl_msg_t)) {

        jack_ringbuffer_read(seq->rb, (char*) &msg, sizeof(sequence_ctrl_msg_t));

        if (msg.param == SEQUENCE_SET_TRIG) {
            sequence_set_trig_now(seq, msg.vi, msg.vp);
        } else if (msg.param == SEQUENCE_CLEAR_TRIG) {
            sequence_clear_trig_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_TRANSPOSE) {
            sequence_set_transpose_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_PH) {
            sequence_set_playhead_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_FIRST) {
            sequence_set_first_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_LAST) {
            sequence_set_last_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_DIV) {
            sequence_set_clockdivide_now(seq, msg.vi);
        } else if (msg.param == SEQUENCE_MUTE) {
            sequence_set_mute_now(seq, msg.vb);
        }

        avail -= sizeof(sequence_ctrl_msg_t);

    }

}

static void notification_data_init(struct notification_data *noti) {

    noti->playhead_new = false;
    noti->playhead = 0;

    noti->first_new = false;
    noti->first = 0;

    noti->last_new = false;
    noti->last = 0;

    noti->transpose_new = false;
    noti->transpose = 0;

    noti->clockdivide_new = false;
    noti->clockdivide = 0;

    noti->mute_new = false;
    noti->mute = 0;


}

