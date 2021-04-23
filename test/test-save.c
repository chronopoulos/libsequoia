#include <unistd.h>

#include "sequoia.h"

#define BPM 120
#define NSTEPS 16

int melody[] = {60, 64, 62, 65, 67, 60, 69, 70,
                    72, 70, 76, 77, 72, 79, 81, 84};

int main(void) {

    // start up a session and set the BPM
    sq_session_t sesh = sq_session_new("mySession");
    sq_session_set_bpm(sesh, BPM);

    // create an outport
    sq_outport_t synthOut = sq_outport_new("synthOut");
    sq_session_register_outport(sesh, synthOut);

    // create a sequence, connect it to the outport
    sq_sequence_t synthSeq = sq_sequence_new(NSTEPS);
    sq_sequence_set_outport(synthSeq, synthOut);

    // populate the sequence with triggers
    sq_trigger_t trig = sq_trigger_new();
    sq_trigger_set_type(trig, TRIG_NOTE);
    for (int i=0; i<NSTEPS; i++) {
        sq_trigger_set_note_value(trig, melody[i]);
        sq_sequence_set_trig(synthSeq, i, trig);
    }
    sq_session_add_sequence(sesh, synthSeq);

    // save sequence to file
    sq_session_save(sesh, "mySession.sqa");

    return 0;

}

