#include "cdc.h"

// startup sequence after power on
uint64_t _SEQ_INIT[] = {0x9F00004ull, 0x9B910100009ull, 0x9920101130009ull, 0x9F00006ull, 0x9F00546ull, 0x9F00546ull, 0x9C1011358037ull, 0x9D0ull, 0x9B910100009ull, 0x9920101130009ull};
t_cdc_sequence SEQ_INIT = {10, 100, _SEQ_INIT};

// response to 0x19
// [2]: [9] = disc
// [5]: [6] = disc
uint64_t _SEQ_START[] = {0x810, 0x9F00544ull, 0x9C1019900000ull, 0x9A000ull, 0x9D0ull, 0x9B91000000Aull, 0x9920001Aull};
t_cdc_sequence SEQ_START = {7, 150, _SEQ_START};

// response to 11181 (set play state: play, resume)
// [1]: [9] = disc
uint64_t _SEQ_ONRESUME[] = {0x9F00544ull, 0x9C1019900000ull, 0x9A000ull};
t_cdc_sequence SEQ_ONRESUME = {3, 150, _SEQ_ONRESUME};

// response to 11101 (set play state: play)
// [3]: [8] = disc
// [7]: disc info
// [9]: [3..4] = track
// [11]: [3..4] = track
uint64_t _SEQ_ONPLAY[] = {0x9D0ull, 0x9F00546ull, 0x9F00546ull, 0x9B910000001ull, 0x99100011ull, 0x99100011ull, 0x9F00546ull, 
  0x9C1011358037ull, 0x9D0ull, 0x99501011ull, 0x9F00546ull, 0x99401011ull};
t_cdc_sequence SEQ_ONPLAY = {12, 150, _SEQ_ONPLAY};

uint64_t _SEQ_CHANGED[] = {0x9B900000001ull, 0x9C001999999Full, 0x994000100000001ull};
t_cdc_sequence SEQ_CHANGED = {3, 50, _SEQ_CHANGED};

uint64_t _SEQ_PING[] = {0x98};
t_cdc_sequence SEQ_PING = {1, 50, _SEQ_PING};


