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

#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <json-c/json.h> 

#include "sequence.h"
#include "outport.h"
#include "inport.h"
#include "offHeap.h"

#define SESSION_MAX_NSEQ 256
#define SESSION_MAX_NINPORTS 16
#define SESSION_MAX_NOUTPORTS 16

struct session_data {

    float bpm; // beats per minute
    bool go;

    int nseqs;
    sq_sequence_t seqs[SESSION_MAX_NSEQ];

    bool is_playing;
    int fps; // frames per step

    jack_client_t *jack_client;
    jack_nframes_t sr; // sample rate
    jack_nframes_t bs; // buffer size
    jack_ringbuffer_t *rb;
    jack_nframes_t frame;

    sq_inport_t inports[SESSION_MAX_NINPORTS];
    sq_outport_t outports[SESSION_MAX_NOUTPORTS];
    size_t ninports, noutports;

    offNode_t **buf_off;    // array of pointers
    size_t len_off;
    size_t idx_off;
    offHeap_t *offHeap;

};

#endif
