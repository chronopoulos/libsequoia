#include <unistd.h>
#include <stdio.h>

#include "sequoia.h"

#define BPM 120
#define NSTEPS 16

int melody[] = {60, 64, 62, 65, 67, 60, 69, 70,
                    72, 70, 76, 77, 72, 79, 81, 84};

int main(void) {

    // start up a session and populate it with data
    sq_session_t sesh = sq_session_new("mySession");
    sq_session_set_bpm(sesh, BPM);
    sq_outport_t synthOut = sq_outport_new("synthOut");
    sq_session_register_outport(sesh, synthOut);
    sq_sequence_t synthSeq = sq_sequence_new(NSTEPS);
    sq_sequence_set_outport(synthSeq, synthOut);
    sq_trigger_t trig = sq_trigger_new();
    sq_trigger_set_type(trig, TRIG_NOTE);
    for (int i=0; i<NSTEPS; i++) {
        sq_trigger_set_note_value(trig, melody[i]);
        sq_sequence_set_trig(synthSeq, i, trig);
    }
    sq_session_add_sequence(sesh, synthSeq);

    // disconnect from jack after waiting for user input
    printf("Press enter to disconnect.");
    getchar();
    sq_session_disconnect_jack(sesh);
    printf("Press enter to exit.");
    getchar();

    return 0;

}

