#ifndef MIDIEVENT_H
#define MIDIEVENT_H

#include <jack/jack.h>

enum midiEventType {MEV_TYPE_NULL, MEV_TYPE_NOTEON, MEV_TYPE_NOTEOFF, MEV_TYPE_CC};

typedef struct {
    enum midiEventType type;
    void *buf;              // JACK MIDI port buffer
    jack_nframes_t time;    // buffer frame index
    jack_nframes_t length;  // length of note (for note-on only)
    unsigned char status;   // MIDI status byte
    unsigned char data1;    // MIDI data byte
    unsigned char data2;    // MIDI data byte
} midiEvent;

void midiEvent_sort(midiEvent*, size_t);

#define MIDIEVENT_NULL (midiEvent) {MEV_TYPE_NULL, NULL, 0, 0, 0, 0, 0}

#endif
