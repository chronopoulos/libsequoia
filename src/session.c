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

            if (sesh->go) {
                for (int i=0; i<sesh->nseqs; i++) {
                    sesh->seqs[i]->is_playing = true;
                }
            } else {
                for (int i=0; i<sesh->nseqs; i++) {
                    sesh->seqs[i]->is_playing = false;
                    _sequence_reset_now(sesh->seqs[i]);
                }
            }

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

static int _process(jack_nframes_t nframes, void *arg) {

    sq_session_t *sesh = (sq_session_t*) arg;

    _session_serve_ctrl_msgs(sesh);

    // serve the inports
    for (int i=0; i<sesh->ninports; i++) {
        _inport_serve(sesh->inports[i], nframes);
    }

    // prepare the outports. need to do this once per port, per processing
    // callback - EVEN IF the session isn't running. otherwise, stopping the
    // sequencer while an outport buffer has a note-on in it will lead to
    // rapidly repeating (machine-gun) note events
    sq_outport_t *outport;
    for (int i=0; i<sesh->noutports; i++) {
        outport = sesh->outports[i];
        outport->buf = jack_port_get_buffer(outport->jack_port, nframes);
        jack_midi_clear_buffer(outport->buf);
    }

    jack_nframes_t nframes_left, frame_inc;

    nframes_left = nframes;
    while(nframes_left) {

        // if we're on a tick boundary, call _tick() on each sequence
        if (sesh->frame == 0) {
            for (int i=0; i<sesh->nseqs; i++) {
                if (sesh->go) {
                    _sequence_tick(sesh->seqs[i], nframes - nframes_left);
                }
                _sequence_serve_off_buffer(sesh->seqs[i], nframes - nframes_left);
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

    return 0;

}

// </helper>

void sq_session_init(sq_session_t *sesh, const char *client_name, int tps) {

    // initialize struct members
    sesh->go = false;
    sesh->tps = tps;
    sesh->nseqs = 0;
    sesh->frame = 0;
    sesh->ninports = 0;
    sesh->noutports = 0;

    // seed random number generator with system time
    srandom(time(NULL));

    // open jack client
    sesh->jack_client = jack_client_open(client_name, JackNoStartServer, NULL);
	if (sesh->jack_client == NULL) {
        fprintf(stderr, "sequoia failed to open JACK client\n");
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

int sq_session_register_outport(sq_session_t *sesh, sq_outport_t *outport) {

    jack_port_t *jack_port;

    if (sesh->noutports == MAX_NOUTPORTS) {
        fprintf(stderr, "max number of outports reached: %d\n", MAX_NOUTPORTS);
        return -1;
    }

    jack_port = jack_port_register(sesh->jack_client, outport->name,
                                    JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

    if (!jack_port) {
        fprintf(stderr, "failed to create JACK port\n");
        return -1;
    }

    outport->jack_client = sesh->jack_client;
    outport->jack_port = jack_port;

    sesh->outports[sesh->noutports] = outport;
    sesh->noutports++;

    return 0;

}

int sq_session_register_inport(sq_session_t *sesh, sq_inport_t *inport) {

    jack_port_t *jack_port;

    if (sesh->ninports == MAX_NINPORTS) {
        fprintf(stderr, "max number of inports reached: %d\n", MAX_NINPORTS);
        return -1;
    }

    jack_port = jack_port_register(sesh->jack_client, inport->name,
                                    JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

    if (!jack_port) {
        fprintf(stderr, "failed to create JACK port\n");
        return -1;
    }

    inport->jack_client = sesh->jack_client;
    inport->jack_port = jack_port;

    sesh->inports[sesh->ninports] = inport;
    sesh->ninports++;

    return 0;

}

void sq_session_start(sq_session_t *sesh) {

    if (!sesh->is_playing) {

        _session_ctrl_msg_t msg;
        msg.param = SESSION_GO;
        msg.vb = true;

        _session_ringbuffer_write(sesh, &msg);

        sesh->is_playing = true;

    }

}

void sq_session_stop(sq_session_t *sesh) {

    if (sesh->is_playing) {

        _session_ctrl_msg_t msg;
        msg.param = SESSION_GO;
        msg.vb = false;

        _session_ringbuffer_write(sesh, &msg);

        sesh->is_playing = false;

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

float sq_session_get_bpm(sq_session_t *sesh) {

    return sesh->bpm;

}

////

json_object *sq_session_get_json(sq_session_t *sesh) {

    json_object *jo_session = json_object_new_object();

    json_object_object_add(jo_session, "name",
                            json_object_new_string(sq_session_get_name(sesh)));

    json_object_object_add(jo_session, "bpm",
                            json_object_new_double(sq_session_get_bpm(sesh)));

    json_object_object_add(jo_session, "tps",
                            json_object_new_int(sq_session_get_tps(sesh)));

    json_object *sequence_array = json_object_new_array();
    for (int i=0; i<sesh->nseqs; i++) {
        json_object_array_add(sequence_array, sq_sequence_get_json(sesh->seqs[i]));
    }
    json_object_object_add(jo_session, "sequences", sequence_array);
    
    return jo_session;

}

void sq_session_save(sq_session_t *sesh, const char *filename) {

    FILE *fp;

    fp = fopen(filename, "w+");
    fprintf(fp, "%s", json_object_to_json_string_ext(sq_session_get_json(sesh),
                                                    JSON_C_TO_STRING_PRETTY));
    fclose(fp);

}

