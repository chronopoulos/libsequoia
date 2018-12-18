#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <jack/midiport.h>

#include "genseq.h"

static int _process(jack_nframes_t nframes, void *arg) {

    struct gs_session_data *sesh = (struct gs_session_data*) arg;

    int avail = jack_ringbuffer_read_space(sesh->rb);
    struct _session_ctrl_msg msg;
    while(avail >= sizeof(struct _session_ctrl_msg)) {

        jack_ringbuffer_read(sesh->rb, (char*) &msg, sizeof(struct _session_ctrl_msg));

        if (msg.param == SESSION_GO) {
            sesh->go = msg.vb;
        }

        avail -= sizeof(struct _session_ctrl_msg);

    }

    void *output_port_buf; // cannot be cached! see jack.h
    if (sesh->go) {

        output_port_buf = jack_port_get_buffer(sesh->jack_port_out, nframes);
        jack_midi_clear_buffer(output_port_buf);

        for (int i=0; i<sesh->nseqs; i++) {
            gs_sequence_process(sesh->seqs[i], nframes, output_port_buf);
        }

    }

    return 0;

}

void gs_session_init(struct gs_session_data *sesh, char *client_name, int bpm, int tps) {

    // initialize struct members
    sesh->go = false;
    sesh->bpm = bpm;
    sesh->tps = tps;
    sesh->nseqs = 0;

    // open jack client
    sesh->jack_client = jack_client_open(client_name, JackNullOption, NULL);
	if (sesh->jack_client == 0) {
        fprintf(stderr, "JACK server not running?\n");
        exit(1);
	}

    // get jack server parameters
    sesh->sr = jack_get_sample_rate(sesh->jack_client);
    sesh->bs = jack_get_buffer_size(sesh->jack_client);

    // calculate frames per tick (fpt), and round to nearest int
    float fpt = ((float)sesh->sr * SECONDS_PER_MINUTE) /
            (sesh->tps * sesh->bpm * STEPS_PER_BEAT);
    sesh->fpt = round(fpt);

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

    int avail = jack_ringbuffer_write_space(sesh->rb);
    if (avail < sizeof(struct _session_ctrl_msg)) {
        fprintf(stderr, "session ringbuffer: overflow\n");
        return;
    }

    struct _session_ctrl_msg msg;
    msg.param = SESSION_GO;
    msg.vb = true;

    jack_ringbuffer_write(sesh->rb, (const char*) &msg, sizeof(struct _session_ctrl_msg));

}

void gs_session_stop(struct gs_session_data *sesh) {

    int avail = jack_ringbuffer_write_space(sesh->rb);
    if (avail < sizeof(struct _session_ctrl_msg)) {
        fprintf(stderr, "session ringbuffer: overflow\n");
        return;
    }

    struct _session_ctrl_msg msg;
    msg.param = SESSION_GO;
    msg.vb = false;

    jack_ringbuffer_write(sesh->rb, (const char*) &msg, sizeof(struct _session_ctrl_msg));

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

