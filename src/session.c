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

#include <stdio.h>
#include <math.h>

#include "sequoia.h"

// <helper>

inline jack_nframes_t _min_nframes(jack_nframes_t a, jack_nframes_t b ) {

    return a < b ? a : b;

}

void _session_ringbuffer_write(sq_session_t *sesh, _session_ctrl_msg_t *msg) {

    int avail = jack_ringbuffer_write_space(sesh->rb);
    if (avail < sizeof(_session_ctrl_msg_t)) {
        fprintf(stderr, "session ringbuffer: overflow\n");
        return;
    }

    jack_ringbuffer_write(sesh->rb, (const char*) msg, sizeof(_session_ctrl_msg_t));

}

void _session_serve_ctrl_msgs(sq_session_t *sesh) {

    int avail = jack_ringbuffer_read_space(sesh->rb);
    _session_ctrl_msg_t msg;
    while(avail >= sizeof(_session_ctrl_msg_t)) {

        jack_ringbuffer_read(sesh->rb, (char*) &msg, sizeof(_session_ctrl_msg_t));

        if (msg.param == SESSION_GO) {

            sesh->go = msg.vb;

        } else if (msg.param == SESSION_BPM) {

            _session_set_bpm_now(sesh, msg.vf);

        } else if (msg.param == SESSION_ADD_SEQ) {

            _session_add_sequence_now(sesh, msg.vp);

        } else if (msg.param == SESSION_RM_SEQ) {

            _session_rm_sequence_now(sesh, msg.vp);

        }

        avail -= sizeof(_session_ctrl_msg_t);

    }

}

// </helper>

static int _process(jack_nframes_t nframes, void *arg) {

    sq_session_t *sesh = (sq_session_t*) arg;

    _session_serve_ctrl_msgs(sesh);

    jack_nframes_t nframes_left, frame_inc;
    if (sesh->go) {

        // do this once per port, per processing callback
        for (int i=0; i<sesh->nseqs; i++) {
            _sequence_prepare_outport(sesh->seqs[i], nframes);
        }

        nframes_left = nframes;
        while(nframes_left) {

            // if we're on a tick boundary, call _tick() on each sequence
            if (sesh->frame == 0) {
                for (int i=0; i<sesh->nseqs; i++) {
                    _sequence_tick(sesh->seqs[i], nframes - nframes_left);
                }
            }

            // increment to the next tick, or as far as we can now
            frame_inc = _min_nframes(sesh->fpt - sesh->frame, nframes_left);
            sesh->frame += frame_inc;
            nframes_left -= frame_inc;
            if (sesh->frame == sesh->fpt) {
                sesh->frame = 0;
            }

        }

    }

    return 0;

}

void sq_session_init(sq_session_t *sesh, const char *client_name, int tps) {

    // initialize struct members
    sesh->go = false;
    sesh->tps = tps;
    sesh->nseqs = 0;
    sesh->frame = 0;

    // seed random number generator with system time
    srandom(time(NULL));

    // open jack client
    sesh->jack_client = jack_client_open(client_name, JackNullOption, NULL);
    
	if (sesh->jack_client == 0) {
        fprintf(stderr, "JACK server not running?\n");
        exit(1);
	}

    // get jack server parameters
    sesh->sr = jack_get_sample_rate(sesh->jack_client);
    sesh->bs = jack_get_buffer_size(sesh->jack_client);

    _session_set_bpm_now(sesh, DEFAULT_BPM);

    // set jack process callback
	jack_set_process_callback(sesh->jack_client, _process, sesh);

    // allocate and lock ringbuffer
    sesh->rb = jack_ringbuffer_create(RINGBUFFER_LENGTH * sizeof(_session_ctrl_msg_t));
    int err = jack_ringbuffer_mlock(sesh->rb);
    if (err) {
        fprintf(stderr, "failed to lock ringbuffer\n");
        exit(1);
    }

    // activate jack client
	if (jack_activate(sesh->jack_client)) {
		fprintf(stderr, "failed to activate client\n");
        exit(1);
	}

    sesh->is_playing = false;

}

jack_port_t *sq_session_create_outport(sq_session_t *sesh, const char *name) {

    jack_port_t *port;

    port = jack_port_register(sesh->jack_client, name, JACK_DEFAULT_MIDI_TYPE,
                                JackPortIsOutput, 0);

    if (!port) {
        fprintf(stderr, "failed to create outport\n");
    }

    return port; // possibly NULL

}

void sq_session_start(sq_session_t *sesh) {

    sesh->is_playing = true;
    for (int i=0; i<sesh->nseqs; i++) {
        sesh->seqs[i]->is_playing = true;
    }

    _session_ctrl_msg_t msg;
    msg.param = SESSION_GO;
    msg.vb = true;

    _session_ringbuffer_write(sesh, &msg);

}

void sq_session_stop(sq_session_t *sesh) {

    sesh->is_playing = false;
    for (int i=0; i<sesh->nseqs; i++) {
        sesh->seqs[i]->is_playing = false;
    }

    _session_ctrl_msg_t msg;
    msg.param = SESSION_GO;
    msg.vb = false;

    _session_ringbuffer_write(sesh, &msg);

    for (int i=0; i<sesh->nseqs; i++) {
        sesh->seqs[i]->is_playing = false;
    }

}

void sq_session_set_bpm(sq_session_t *sesh, float bpm) {

    if (sesh->is_playing) {

        _session_ctrl_msg_t msg;
        msg.param = SESSION_BPM;
        msg.vf = bpm;

        _session_ringbuffer_write(sesh, &msg);

    } else {

        _session_set_bpm_now(sesh, bpm);

    }

}

void _session_set_bpm_now(sq_session_t *sesh, float bpm) {

    sesh->bpm = bpm;

    // calculate frames per tick (fpt) (no need to cast becasue sesh->bpm is a float)
    float fpt = (sesh->sr * SECONDS_PER_MINUTE) / (sesh->tps * sesh->bpm * STEPS_PER_BEAT);

    // and round to nearest int
    sesh->fpt = round(fpt);

    // if an increase in tempo has lowered the fpt below the current frame index,
    //  then reset the frame index to avoid a runaway frame count
    // (note that we could also achieve this using (sesh->frame >= sesh->fpt) in _process(),
    //  but this method ensures that we don't skip a beat on tempo increases
    if (sesh->frame >= sesh->fpt) {
        sesh->frame = 0;
    }

}

void sq_session_add_sequence(sq_session_t *sesh, sq_sequence_t *seq) {

    if (seq->tps != sesh->tps) {
        fprintf(stderr, "seq->tps doesn't match sesh->tps: %d vs %d\n", seq->tps, sesh->tps);
        return;
    }

    if (seq->outport == NULL) {
        fprintf(stderr, "cannot add sequence with NULL outport\n");
        return;
    }

    if (sesh->is_playing) {

        _session_ctrl_msg_t msg;
        msg.param = SESSION_ADD_SEQ;
        msg.vp = seq;

        _session_ringbuffer_write(sesh, &msg);

    } else {

        _session_add_sequence_now(sesh, seq);

    }

}

void _session_add_sequence_now(sq_session_t *sesh, sq_sequence_t *seq) {

    sesh->seqs[sesh->nseqs] = seq;
    sesh->nseqs++;

}

void sq_session_rm_sequence(sq_session_t *sesh, sq_sequence_t *seq) {

    // NOTE: this does not free the memory pointed to by seq;
    //  the caller must do that explicitly

    if (sesh->is_playing) {

        _session_ctrl_msg_t msg;
        msg.param = SESSION_RM_SEQ;
        msg.vp = seq;

        _session_ringbuffer_write(sesh, &msg);

    } else {

        _session_rm_sequence_now(sesh, seq);

    }

}

void _session_rm_sequence_now(sq_session_t *sesh, sq_sequence_t *seq) {

    int i;

    for (i=0; i<sesh->nseqs; i++) {
        if (sesh->seqs[i] == seq) {
            break;
        }
    }

    if (i < sesh->nseqs) { // then we found it at i
        sesh->nseqs--; // decrement nseqs
        for (; i<sesh->nseqs; i++) {
            sesh->seqs[i] = sesh->seqs[i+1]; // left-shift the tail of the vector
        }
    } // else do nothing

}

char *sq_session_get_name(sq_session_t *sesh) {

    return jack_get_client_name(sesh->jack_client);

}

// read-only getters don't need to use ringbuffers

int sq_session_get_tps(sq_session_t *sesh) {

    return sesh->tps;

}

int sq_session_get_bpm(sq_session_t *sesh) {

    return sesh->bpm;

}
