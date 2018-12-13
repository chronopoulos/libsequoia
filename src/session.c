#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <jack/midiport.h>

#include "ziggurat/session.h"

static int _process(jack_nframes_t nframes, void *arg) {

    struct zig_session_data *sesh;
    void *output_port_buf; // cannot be cached! see jack.h

    sesh = (struct zig_session_data*) arg;

    if (sesh->go) {

        output_port_buf = jack_port_get_buffer(sesh->jack_port_out, nframes);
        jack_midi_clear_buffer(output_port_buf);

        for (int i=0; i<sesh->nseqs; i++) {
            zig_sequence_process(sesh->seqs[i], nframes, output_port_buf);
        }

    }

    return 0;

}

void zig_session_init(struct zig_session_data *sesh, int bpm, int tps) {

    // initialize struct members
    sesh->go = false;
    sesh->bpm = bpm;
    sesh->tps = tps;
    sesh->nseqs = 0;

    // open jack client
    sesh->jack_client = jack_client_open("ziggurat_client", JackNullOption, NULL);
	if (sesh->jack_client == 0) {
        fprintf (stderr, "JACK server not running?\n");
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
        fprintf (stderr, "could not register jack port\n");
    }

    // activate jack client
	if (jack_activate(sesh->jack_client)) { // this makes it pop up in carla, at least
		fprintf (stderr, "cannot activate client");
	}

}

void zig_session_start(struct zig_session_data *sesh) {

    sesh->go = true;

}

void zig_session_stop(struct zig_session_data *sesh) {

    sesh->go = false;

}

void zig_session_add_sequence(struct zig_session_data *sesh, struct zig_sequence_data *seq) {

    sesh->seqs[sesh->nseqs] = seq;
    sesh->nseqs++;

}

void zig_session_wait(struct zig_session_data *sesh) {

    while(1) {
        usleep(1000);
    }

}

