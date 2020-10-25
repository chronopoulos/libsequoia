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
#include <string.h>

#include "sequoia.h"
#include "sequoia/midiEvent.h"

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

void _session_reset_frame_counter(sq_session_t *sesh) {

    sesh->frame = sesh->fps / 2;    // frame counter

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
                _session_reset_frame_counter(sesh);
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
    offNode_t *offp, **offpp;   // tmp vars

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

    // main processing for midi output

    jack_nframes_t nframes_left, len;
    unsigned char *midi_msg_write_ptr;

    midiEvent mevs[MAX_NSEQ];
    size_t len_mevs = 0;
    midiEvent mev;     // midiEvent temp variable
    midiEvent *mevp;   // midiEvent* temp variable

    if (sesh->go) {

        // collect mevs (note-on and CC) for this processing block
        nframes_left = nframes;
        while(nframes_left) {
            len = _min_nframes(nframes_left, sesh->fps - sesh->frame);
                for (int i=0; i<sesh->nseqs; i++) {

                    if (sesh->frame == 0) _sequence_step(sesh->seqs[i]);
                    mev = _sequence_process(sesh->seqs[i], sesh->fps,
                                        sesh->frame, len, nframes - nframes_left);
                    if (mev.buf) mevs[len_mevs++] = mev;    // check for NULL
                    if (mev.type == MEV_TYPE_NOTEON) {      // queue note off
                        // allocate and set offNode
                        offp = offHeap_alloc(sesh->offHeap);
                        offp->mev.type = MEV_TYPE_NOTEOFF;
                        offp->mev.buf = mev.buf;
                        offp->mev.status = mev.status - 16; // convert on to off
                        offp->mev.data1 = mev.data1;
                        offp->next = NULL;
                        // add it to the linked-list array that is buf_off
                        offpp = sesh->buf_off + ((sesh->idx_off + mev.time + mev.length) % sesh->len_off);
                        while (*offpp) {   // follow the linked list until next == NULL
                            offpp = &((*offpp)->next);
                        }
                        (*offpp) = offp;
                    }
                    
                }
            sesh->frame += len;
            if (sesh->frame == sesh->fps) sesh->frame = 0;
            nframes_left -= len;
        }

    }

    // cycle through note-off buffer, collect them as mevs
    for (size_t i=0; i<nframes; i++) {
        offp = sesh->buf_off[(sesh->idx_off + i) % sesh->len_off];
        while (offp) {
            mevs[len_mevs] = offp->mev;
            mevs[len_mevs++].time = i;
            offHeap_free(sesh->offHeap, offp);
            offp = offp->next;
        }
        sesh->buf_off[(sesh->idx_off + i) % sesh->len_off] = NULL;  // could do with an offpp?
    }
    sesh->idx_off = (sesh->idx_off + nframes) % sesh->len_off;

    // sort all mevs
    midiEvent_sort(mevs, len_mevs);

    // fire off mevs
    for (size_t i=0; i<len_mevs; i++) {
        mevp = mevs + i;
        midi_msg_write_ptr = jack_midi_event_reserve(mevp->buf, mevp->time, 3);
        midi_msg_write_ptr[0] = mevp->status;
        midi_msg_write_ptr[1] = mevp->data1;
        midi_msg_write_ptr[2] = mevp->data2;
    }

    return 0;

}

// </helper>

sq_session_t *sq_session_new(const char *client_name) {

    sq_session_t *sesh;

    sesh = malloc(sizeof(sq_session_t));

    // initialize struct members
    sesh->go = false;
    sesh->nseqs = 0;
    sesh->ninports = 0;
    sesh->noutports = 0;
    sesh->is_playing = false;

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

    _session_set_bpm_now(sesh, DEFAULT_BPM);    // this also sets fps

    _session_reset_frame_counter(sesh);

    // set jack process callback
	jack_set_process_callback(sesh->jack_client, _process, sesh);

    // allocate and lock ringbuffer
    sesh->rb = jack_ringbuffer_create(RINGBUFFER_LENGTH * sizeof(_session_ctrl_msg_t));
    int err = jack_ringbuffer_mlock(sesh->rb);
    if (err) {
        fprintf(stderr, "failed to lock ringbuffer\n");
        exit(1);
    }

    // allocate and initialize note-off buffer, plus offHeap
    sesh->len_off = sesh->fps * TRIG_MAX_LENGTH;
    sesh->buf_off = malloc(sizeof(offNode_t*) * sesh->len_off);
    for (size_t i=0; i<sesh->len_off; i++) {
        sesh->buf_off[i] = NULL;
    }
    sesh->idx_off = 0;
    sesh->offHeap = offHeap_new(MAX_NSEQ * MAX_SEQ_NSTEPS);

    // activate jack client
	if (jack_activate(sesh->jack_client)) {
		fprintf(stderr, "failed to activate client\n");
        exit(1);
	}

    return sesh;

}


void sq_session_disconnect_jack(sq_session_t *sesh) {

    if (jack_client_close(sesh->jack_client)) {
        fprintf(stderr, "sequoia failed to disconnect jack client\n");
    }

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

sq_outport_t *sq_session_get_outport_from_name(sq_session_t *sesh, const char *name) {

    for (int i=0; i<sesh->noutports; i++) {
        if (strcmp(sesh->outports[i]->name, name) == 0) {
            return sesh->outports[i];
        }
    }

    return NULL;

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

sq_inport_t *sq_session_get_inport_from_name(sq_session_t *sesh, const char *name) {

    for (int i=0; i<sesh->ninports; i++) {
        if (strcmp(sesh->inports[i]->name, name) == 0) {
            return sesh->inports[i];
        }
    }

    return NULL;

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

    // calculate frames per step (fps) (no need to cast because sesh->bpm is a float)
    float fps = (sesh->sr * SECONDS_PER_MINUTE) / (sesh->bpm * STEPS_PER_BEAT);

    // and round to nearest int
    sesh->fps = round(fps);

    // if an increase in tempo has lowered the fps below the current frame index,
    //  then reset the frame index to avoid a runaway frame count
    // (note that we could also achieve this using (sesh->frame >= sesh->fps) in _process(),
    //  but this method ensures that we don't skip a beat on tempo increases)
    // HOWEVER it's worth considering: would we *rather* skip a beat?
    if (sesh->frame >= sesh->fps) {
        sesh->frame = 0;
    }

}

void sq_session_add_sequence(sq_session_t *sesh, sq_sequence_t *seq) {

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

sq_sequence_t *sq_session_get_sequence_from_name(sq_session_t *sesh, const char *name) {

    for (int i=0; i<sesh->nseqs; i++) {
        if (strcmp(sesh->seqs[i]->name, name) == 0) {
            return sesh->seqs[i];
        }
    }

    return NULL;

}

char *sq_session_get_name(sq_session_t *sesh) {

    return jack_get_client_name(sesh->jack_client);

}

// read-only getters don't need to use ringbuffers

float sq_session_get_bpm(sq_session_t *sesh) {

    return sesh->bpm;

}

////

json_object *sq_session_get_json(sq_session_t *sesh) {

    json_object *jo_session = json_object_new_object();

    // top-level attributes
    json_object_object_add(jo_session, "name",
                            json_object_new_string(sq_session_get_name(sesh)));
    json_object_object_add(jo_session, "bpm",
                            json_object_new_double(sq_session_get_bpm(sesh)));

    // sequences
    json_object *sequence_array = json_object_new_array();
    for (int i=0; i<sesh->nseqs; i++) {
        json_object_array_add(sequence_array, sq_sequence_get_json(sesh->seqs[i]));
    }
    json_object_object_add(jo_session, "sequences", sequence_array);
    

    // inports
    json_object *inport_array = json_object_new_array();
    for (int i=0; i<sesh->ninports; i++) {
        json_object_array_add(inport_array, sq_inport_get_json(sesh->inports[i]));
    }
    json_object_object_add(jo_session, "inports", inport_array);
    
    // outports
    json_object *outport_array = json_object_new_array();
    for (int i=0; i<sesh->noutports; i++) {
        json_object_array_add(outport_array, sq_outport_get_json(sesh->outports[i]));
    }
    json_object_object_add(jo_session, "outports", outport_array);
    
    return jo_session;

}

void sq_session_save(sq_session_t *sesh, const char *filename) {

    FILE *fp;

    fp = fopen(filename, "w+");
    fprintf(fp, "%s", json_object_to_json_string_ext(sq_session_get_json(sesh),
                                                    JSON_C_TO_STRING_PRETTY));
    fclose(fp);

}

sq_session_t *sq_session_load(const char *filename) {

    FILE *fp;
    long filesize;
    struct json_object *jo_session;
    char *buf;
    size_t ret;
    sq_session_t *sesh = NULL;

    // open the file descriptor
    fp = fopen(filename, "r");

    // get the file size
    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    rewind(fp);

    // allocate the read buffer
    buf = malloc(filesize);

    // read the file into the buffer, parse and instantiate the session
    ret = fread(buf, 1, filesize, fp);
    if (ret == filesize) {
        jo_session = json_tokener_parse(buf);
        sesh = sq_session_malloc_from_json(jo_session);
    } else {
        fprintf(stderr, "error while reading file: %s", filename);
    }

    // clean up
    fclose(fp);
    free(buf);

    return sesh;

}

sq_session_t *sq_session_malloc_from_json(json_object *jo_session) {

    struct json_object *jo_tmp, *jo_tmp2, *jo_tmp3, *jo_tmp4;
    const char *name;
    double bpm;
    sq_session_t *sesh;
    sq_sequence_t *seq_tmp = NULL;
    sq_inport_t *inport_tmp = NULL;
    sq_outport_t *outport_tmp = NULL;

    // first extract the top-level attributes
    json_object_object_get_ex(jo_session, "name", &jo_tmp);
    name = json_object_get_string(jo_tmp);
    json_object_object_get_ex(jo_session, "bpm", &jo_tmp);
    bpm = json_object_get_double(jo_tmp);

    // malloc and init the session
    sesh = sq_session_new(name);
    sq_session_set_bpm(sesh, bpm);

    // add the outports
    json_object_object_get_ex(jo_session, "outports", &jo_tmp);
    for (int i=0; i<json_object_array_length(jo_tmp); i++) {
        jo_tmp2 = json_object_array_get_idx(jo_tmp, i);
        outport_tmp = sq_outport_malloc_from_json(jo_tmp2);
        sq_session_register_outport(sesh, outport_tmp);
    }

    // add the sequences
    json_object_object_get_ex(jo_session, "sequences", &jo_tmp);
    for (int i=0; i<json_object_array_length(jo_tmp); i++) {
        jo_tmp2 = json_object_array_get_idx(jo_tmp, i);
        seq_tmp = sq_sequence_malloc_from_json(jo_tmp2);
        // outport remains null, resolve it now
        json_object_object_get_ex(jo_tmp2, "outport", &jo_tmp3);
        if (json_object_get_type(jo_tmp3) == json_type_string) {
            name = json_object_get_string(jo_tmp3);
            outport_tmp = sq_session_get_outport_from_name(sesh, name);
            if (outport_tmp) {
                sq_sequence_set_outport(seq_tmp, outport_tmp);
            }
        }
        sq_session_add_sequence(sesh, seq_tmp);
    }

    // add the inports
    json_object_object_get_ex(jo_session, "inports", &jo_tmp);
    for (int i=0; i<json_object_array_length(jo_tmp); i++) {
        jo_tmp2 = json_object_array_get_idx(jo_tmp, i);
        inport_tmp = sq_inport_malloc_from_json(jo_tmp2);
        // seqs remains null, resolve them now
        json_object_object_get_ex(jo_tmp2, "sequences", &jo_tmp3);
        for (int j=0; j<json_object_array_length(jo_tmp3); j++) {
            jo_tmp4 = json_object_array_get_idx(jo_tmp3, i);
            name = json_object_get_string(jo_tmp4);
            seq_tmp = sq_session_get_sequence_from_name(sesh, name);
            if (seq_tmp) {
                sq_inport_add_sequence(inport_tmp, seq_tmp);
            }
        }
        sq_session_register_inport(sesh, inport_tmp);
    }

    return sesh;

}

void sq_session_delete(sq_session_t *sesh) {

    // frees the sq_session_t struct (but not its sequences, ports, etc)

    offHeap_delete(sesh->offHeap);
    free(sesh->buf_off);
    free(sesh);

}

void sq_session_teardown(sq_session_t *sesh) {

    // recursively frees all the malloc'd memory attributed to the session

    sq_session_stop(sesh);
    sq_session_disconnect_jack(sesh);

    // sequences
    for (int i=0; i<sesh->nseqs; i++) {
        sq_sequence_delete(sesh->seqs[i]);
    }

    // inports
    for (int i=0; i<sesh->ninports; i++) {
        sq_inport_delete(sesh->inports[i]);
    }

    // outports
    for (int i=0; i<sesh->noutports; i++) {
        sq_outport_delete(sesh->outports[i]);
    }

    // session
    sq_session_delete(sesh);

}

