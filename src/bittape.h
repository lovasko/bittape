// Copyright (c) 2020 Daniel Lovasko
// All Rights Reserved
//
// Distributed under the terms of the 2-clause BSD License. The full
// license is in the file LICENSE, distributed as part of this software.

#ifndef BITTAPE_H
#define BITTAPE_H

#include <stdint.h>
#include <stdbool.h>


// Select the appropriate word type.
#if   BITTAPE_BIT == 8
  #define BITTAPE_WORD uint8_t
#elif BITTAPE_BIT == 16
  #define BITTAPE_WORD uint16_t
#elif BITTAPE_BIT == 32
  #define BITTAPE_WORD uint32_t
#elif BITTAPE_BIT == 64
  #define BITTAPE_WORD uint64_t
#else
  #error "BITTAPE_BIT has unsupported value"
#endif

// Select the appropriate length type.
#ifndef BITTAPE_LEN
  #if   BITTAPE_BIT == 8
    #define BITTAPE_LEN uint8_t
  #elif BITTAPE_BIT == 16
    #define BITTAPE_LEN uint16_t
  #elif BITTAPE_BIT == 32
    #define BITTAPE_LEN uint32_t
  #elif BITTAPE_BIT == 64
    #define BITTAPE_LEN uint64_t
  #else
    #error "BITTAPE_BIT has unsupported value"
  #endif
#endif

/// Bit tape.
struct bittape {
  BITTAPE_LEN bt_get;              ///< Reading position in bits.
  BITTAPE_LEN bt_put;              ///< Writing position in bits.
  BITTAPE_LEN bt_max;              ///< Maximal number of bits.
  BITTAPE_WORD bt_buf[BITTAPE_BUF]; ///< Bits storage.
};

// Operations.
void bittape_new(struct bittape* tape, const BITTAPE_LEN max);
bool bittape_put(struct bittape* tape, const BITTAPE_LEN cnt, const BITTAPE_WORD val);
bool bittape_get(struct bittape* tape, const BITTAPE_LEN cnt, BITTAPE_WORD* val);

#endif
