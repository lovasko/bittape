// Copyright (c) 2020 Daniel Lovasko
// All Rights Reserved
//
// Distributed under the terms of the 2-clause BSD License. The full
// license is in the file LICENSE, distributed as part of this software.

#include <limits.h>

#include "bittape.h"


/// Initialise the bit tape.
/// @return success/failure indication
///
/// @param[in] tape bit tape
/// @param[in] len  maximal number of bits
void
bittape_new(struct bittape* tape, const BITTAPE_LEN max)
{
  // Reset both heads.
  tape->bt_get = 0;
  tape->bt_put = 0;

  // Set the maximum length. It is also expected that the array that stores
  // the actual bits is already allocated with the appropriate size, so that
  // the maximum number of bits can be appended.
  tape->bt_max = max;
}

/// Perform a write within a single word.
///
/// @param[in] tape bit tape
/// @param[in] cnt  number of bits
/// @param[in] val  word to write
static void
put(struct bittape* tape, const BITTAPE_LEN cnt, const BITTAPE_WORD val)
{
  BITTAPE_LEN idx;
  BITTAPE_LEN off;

  // Compute the current writing word index.
  idx = tape->bt_put / BITTAPE_BIT;

  // Compute the offset within the word.
  off = tape->bt_put % BITTAPE_BIT;

  // Write the requested bits.
  tape->bt_buf[idx] |= ((BITTAPE_WORD)val << off);

  // Advance the writing head.
  tape->bt_put += cnt;
}

/// Write bits to the tape at the writing head position.
/// @return success/failure indication
///
/// @param[in] tape bit tape
/// @param[in] len  number of bits to write
/// @param[in] val  word to write
bool
bittape_put(struct bittape* tape, const BITTAPE_LEN cnt, const BITTAPE_WORD val)
{
  BITTAPE_LEN spl; // Split index between the two words.
  BITTAPE_LEN pos; // Bit index within the current word.
  BITTAPE_LEN adj; // Adjusted bit index after the write.

  // Ensure that the tape has sufficient space for the requested bits.
  if (tape->bt_put + cnt > tape->bt_max) {
    return false;
  }

  // Determine where the split ought to be.
  pos =  tape->bt_put        % BITTAPE_BIT;
  adj = (tape->bt_put + cnt) % BITTAPE_BIT;
  spl = adj * (adj <= pos);

  // Perform the first write which always occurs.
  put(tape, cnt - spl, val);

  // Only perform the second write if any bits fall onto the second word.
  if (spl > 0) {
    put(tape, spl, val >> (cnt - spl));
  }

  return true;
}

/// Perform a read within a single word.
/// @return value
///
/// @param[in] tape bit tape
/// @param[in] cnt  number of bits
static BITTAPE_WORD
get(struct bittape* tape, const BITTAPE_LEN cnt)
{
  BITTAPE_LEN  idx;
  BITTAPE_LEN  off;
  BITTAPE_WORD val;

  // Compute the current reading word index.
  idx = tape->bt_get / BITTAPE_BIT;

  // Compute the offset within the word.
  off = tape->bt_get % BITTAPE_BIT;

  // Retrieve the value at the selected offset.
  val = tape->bt_buf[idx] >> off;

  // Remove all unwanted higher bits.
  if (cnt < BITTAPE_BIT) {
    val &= ((BITTAPE_WORD)1 << cnt) - 1;
  }

  // Advance the reading head.
  tape->bt_get += cnt;

  return val;
}

/// Read bits from the tape.
/// @return success/failure
///
/// @param[in]  tape bit tape
/// @param[in]  cnt  number of bits to read
/// @param[out] val  read bits
bool
bittape_get(struct bittape* tape, const BITTAPE_LEN cnt, BITTAPE_WORD* val)
{
  BITTAPE_LEN spl;
  BITTAPE_LEN pos;
  BITTAPE_LEN adj;

  // Ensure that the tape has requested number of bits available.
  if (tape->bt_get + cnt > tape->bt_put) {
    return false;
  }

  // Determine where the split ought to be.
  pos =  tape->bt_get        % BITTAPE_BIT;
  adj = (tape->bt_get + cnt) % BITTAPE_BIT;
  spl = adj * (adj <= pos);

  // Perform two reads, one for each part.
  *val = get(tape, cnt - spl);

  // Only perform the second read if the request spans two words.
  if (spl > 0) {
    *val |= (get(tape, spl) << (cnt - spl));
  }

  return true;
}
