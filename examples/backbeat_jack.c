#include "ziggurat.h"

void fillEveryN(struct zig_sequence_data *seq, int n, int note) {

    int i;
    for (i=0; i<seq->length; i++) {
        if ((i%n) == 0) {
            zig_sequence_set_trig(seq, i, note);
        } else {
            zig_sequence_set_trig(seq, i, 0);
        }
    }

}

int main(void) {

    struct zig_session_data sesh;
    zig_session_init(&sesh, ZIGGURAT_TYPE_JACK);


    struct zig_sequence_data seq1, seq2, seq3;

    zig_sequence_init(&seq1, 16);
    fillEveryN(&seq1, 4, 60);
    zig_session_add_sequence(&sesh, &seq1);

    zig_sequence_init(&seq2, 16);
    fillEveryN(&seq2, 8, 61);
    zig_session_add_sequence(&sesh, &seq2);

    zig_sequence_init(&seq3, 16);
    fillEveryN(&seq3, 1, 62);
    zig_session_add_sequence(&sesh, &seq3);


    zig_session_start(&sesh);
    zig_session_wait(&sesh);

    return 0;

}

