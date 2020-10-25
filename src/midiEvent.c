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

#include "sequoia/midiEvent.h"

static void _merge(midiEvent *arr, size_t lenL, size_t lenR) {

    // L goes from arr[0,lenL)
    // R goes from arr[lenL,lenL+lenR)

    size_t i, iL, iR;
    midiEvent L[lenL], R[lenR];
    midiEvent tvL, tvR;   // temp values

    // copy into local temp arrays
    for (i=0; i<lenL; i++) {
        L[i] = arr[i];
    }
    for (i=0; i<lenR; i++) {
        R[i] = arr[lenL + i];
    }

    // fill the original array with ordered data from the temp arrays
    iL=0;
    iR=0;
    for (i=0; i<(lenL+lenR); i++) {

        if (iL >= lenL) {
            arr[i] = R[iR];
            iR++;
        } else if (iR >= lenR) {
            arr[i] = L[iL];
            iL++;
        } else {
            tvL = L[iL];
            tvR = R[iR];
            if (tvL.time <= tvR.time) {
                arr[i] = tvL;
                iL++;
            } else {
                arr[i] = tvR;
                iR++;
            }
        }

    }

}

void midiEvent_sort(midiEvent *arr, size_t len) {

    size_t lenL, lenR;

    if (len <= 1) return;

    lenL = len / 2;
    lenR = len - lenL;

    midiEvent_sort(arr, lenL);
    midiEvent_sort(arr + lenL, lenR);

    _merge(arr, lenL, lenR);

}

