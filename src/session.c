#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <jack/midiport.h>

#include "ziggurat/session.h"

static void *_worker_session(void *arg) {

    struct zig_session_data *sesh = (struct zig_session_data*) arg;

    struct timespec time;
    double lastTick = 0.;
    int i;

    while(1) {

        pthread_mutex_lock(&sesh->mtx_go);
        if (sesh->go) {
            pthread_mutex_unlock(&sesh->mtx_go);
            clock_gettime(CLOCK_MONOTONIC, &time);
            if ((time.tv_sec + time.tv_nsec/1e9 - lastTick) >= sesh->period_sec) {
                lastTick = time.tv_sec + time.tv_nsec/1e9;
                for (i=0; i<(sesh->nseqs); i++) {
                    pthread_mutex_lock(&sesh->seqs[i]->mtx_tickFlag);
                    sesh->seqs[i]->tickFlag = true;
                    pthread_mutex_unlock(&sesh->seqs[i]->mtx_tickFlag);
                    pthread_cond_signal(&sesh->seqs[i]->cond_tickFlag);
                }
            }
        } else {
            pthread_mutex_unlock(&sesh->mtx_go);
        }
        usleep(1000); // 1 ms resolution (to keep CPU usage down)

    }

    return NULL;

}

static int _jack_callback(jack_nframes_t nframes, void *arg) {

    struct zig_session_data *sesh = (struct zig_session_data*) arg;

    void *output_port_buf; // cannot be cached! see jack.h
    output_port_buf = jack_port_get_buffer(sesh->jack_port_out, nframes);
	jack_midi_clear_buffer(output_port_buf);

    size_t space;
    char midi_msg_buf[3];
    unsigned char *midi_msg_out_ptr;
	while ((space = jack_ringbuffer_read_space(sesh->rbo))) {
        jack_ringbuffer_read(sesh->rbo, midi_msg_buf, 3);
		midi_msg_out_ptr = jack_midi_event_reserve(output_port_buf, 0, 3);
        memcpy(midi_msg_out_ptr, midi_msg_buf, 3);
    }

    return 0;

}

void zig_session_init(struct zig_session_data *sesh, int type) {

    // initialize struct mmembers
    sesh->go = false;
    sesh->period_sec = PERIOD_SEC_DEFAULT;
    sesh->nseqs = 0;

    // open jack client
    sesh->jack_client = jack_client_open("ziggurat_client", JackNullOption, NULL);
	if (sesh->jack_client == 0) {
        fprintf (stderr, "JACK server not running?\n");
	}

    // set jack process callback
	jack_set_process_callback(sesh->jack_client, _jack_callback, sesh);

    // register jack MIDI output port
    sesh->jack_port_out = jack_port_register(sesh->jack_client, "out1", JACK_DEFAULT_MIDI_TYPE,
                                            JackPortIsOutput, 0);
    if (sesh->jack_port_out == NULL) {
        fprintf (stderr, "could not register jack port\n");
    }

    // create the ringbuffer associated with jack_port_out, and lock it to memory
	sesh->rbo = jack_ringbuffer_create(RINGBUFFER_SIZE);
	if (sesh->rbo == NULL) {
		fprintf(stderr, "Cannot create JACK ringbuffer\n");
	}
	jack_ringbuffer_mlock(sesh->rbo);

    // activate jack client
	if (jack_activate(sesh->jack_client)) { // this makes it pop up in carla, at least
		fprintf (stderr, "cannot activate client");
	}

    pthread_mutex_init(&sesh->mtx_go, NULL);
    pthread_create(&sesh->thread, NULL, _worker_session, sesh);

}

void zig_session_start(struct zig_session_data *sesh) {

    pthread_mutex_lock(&sesh->mtx_go);
    sesh->go = true;
    pthread_mutex_unlock(&sesh->mtx_go);

}

void zig_session_stop(struct zig_session_data *sesh) {

    pthread_mutex_lock(&sesh->mtx_go);
    sesh->go = false;
    pthread_mutex_unlock(&sesh->mtx_go);

}

void zig_session_add_sequence(struct zig_session_data *sesh, struct zig_sequence_data *seq) {

    zig_sequence_set_rbo(seq, sesh->rbo);
    sesh->seqs[sesh->nseqs] = seq;
    sesh->nseqs++;

}

void zig_session_wait(struct zig_session_data *sesh) {

    void *retval;
    pthread_join(sesh->thread, &retval);

}

