#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <jack/midiport.h>

#include "genseq.h"

// <helper>

inline jack_nframes_t _min_nframes(jack_nframes_t a, jack_nframes_t b ) {

    return a < b ? a : b;

}

void _session_ringbuffer_write(struct gs_session_data *sesh, struct _session_ctrl_msg *msg) {

    int avail = jack_ringbuffer_write_space(sesh->rb);
    if (avail < sizeof(struct _session_ctrl_msg)) {
        fprintf(stderr, "session ringbuffer: overflow\n");
        return;
    }

    jack_ringbuffer_write(sesh->rb, (const char*) msg, sizeof(struct _session_ctrl_msg));

}

void _session_handle_bpm_change(struct gs_session_data *sesh) {

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

// </helper>

static int _process(jack_nframes_t nframes, void *arg) {

    struct gs_session_data *sesh = (struct gs_session_data*) arg;

    int avail = jack_ringbuffer_read_space(sesh->rb);
    struct _session_ctrl_msg msg;
    while(avail >= sizeof(struct _session_ctrl_msg)) {

        jack_ringbuffer_read(sesh->rb, (char*) &msg, sizeof(struct _session_ctrl_msg));

        if (msg.param == SESSION_GO) {
            sesh->go = msg.vb;
        } else if (msg.param == SESSION_BPM) {
            sesh->bpm = msg.vf;
            _session_handle_bpm_change(sesh);
        }

        avail -= sizeof(struct _session_ctrl_msg);

    }

    void *output_port_buf; // cannot be cached! see jack.h
    output_port_buf = jack_port_get_buffer(sesh->jack_port_out, nframes);
    jack_midi_clear_buffer(output_port_buf);

    jack_nframes_t nframes_left, frame_inc;
    if (sesh->go) {

        nframes_left = nframes;
        while(nframes_left) {

            // if we're on a tick boundary, call _tick() on each sequence
            if (sesh->frame == 0) {
                for (int i=0; i<sesh->nseqs; i++) {
                    gs_sequence_tick(sesh->seqs[i], output_port_buf, nframes - nframes_left);
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

void gs_session_init(struct gs_session_data *sesh, char *client_name, int tps) {

    // initialize struct members
    sesh->go = false;
    sesh->bpm = DEFAULT_BPM;
    sesh->tps = tps;
    sesh->nseqs = 0;
    sesh->frame = 0;

    // open jack client
    sesh->jack_client = jack_client_open(client_name, JackNullOption, NULL);
	if (sesh->jack_client == 0) {
        fprintf(stderr, "JACK server not running?\n");
        exit(1);
	}

    // get jack server parameters
    sesh->sr = jack_get_sample_rate(sesh->jack_client);
    sesh->bs = jack_get_buffer_size(sesh->jack_client);

    sesh->bpm = DEFAULT_BPM;
    _session_handle_bpm_change(sesh);

    // set jack process callback
	jack_set_process_callback(sesh->jack_client, _process, sesh);

    // register jack MIDI output port
    sesh->jack_port_out = jack_port_register(sesh->jack_client, "out1", JACK_DEFAULT_MIDI_TYPE,
                                            JackPortIsOutput, 0);
    if (sesh->jack_port_out == NULL) {
        fprintf(stderr, "failed to register jack port\n");
        exit(1);
    }

    // allocate and lock ringbuffer
    sesh->rb = jack_ringbuffer_create(RINGBUFFER_LENGTH * sizeof(struct _session_ctrl_msg));
    int err = jack_ringbuffer_mlock(sesh->rb);
    if (err) {
        fprintf(stderr, "failed to lock ringbuffer\n");
        exit(1);
    }

    // activate jack client
	if (jack_activate(sesh->jack_client)) { // this makes it pop up in carla, at least
		fprintf(stderr, "failed to activate client\n");
        exit(1);
	}

}

void gs_session_start(struct gs_session_data *sesh) {

    struct _session_ctrl_msg msg;
    msg.param = SESSION_GO;
    msg.vb = true;

    _session_ringbuffer_write(sesh, &msg);

}

void gs_session_stop(struct gs_session_data *sesh) {

    struct _session_ctrl_msg msg;
    msg.param = SESSION_GO;
    msg.vb = false;

    _session_ringbuffer_write(sesh, &msg);

}

void gs_session_set_bpm(struct gs_session_data *sesh, float bpm) {

    struct _session_ctrl_msg msg;
    msg.param = SESSION_BPM;
    msg.vf = bpm;

    _session_ringbuffer_write(sesh, &msg);

}

void gs_session_add_sequence(struct gs_session_data *sesh, struct gs_sequence_data *seq) {

    sesh->seqs[sesh->nseqs] = seq;
    sesh->nseqs++;

}

void gs_session_wait(struct gs_session_data *sesh) {

    while(1) {
        usleep(1000);
    }

}

