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
/// @param[in] bit bit tape
/// @param[in] len maximal number of bits
void
bittape_new(struct bittape* bit, const BITTAPE_LEN max)
{
  // Reset both heads.
  bit->bt_get = 0;
  bit->bt_put = 0;

  // Set the maximum length. It is also expected that the array that stores
  // the actual bits is already allocated with the appropriate size, so that
  // the maximum number of bits can be appended.
  bit->bt_max = max;
}

#define xstr(x) str(x)
#define str(x) #x

/// Perform a write within a single word.
///
/// @param[in] bit bit tape
/// @param[in] val value to write
/// @param[in] cnt number of bits
static void
put(struct bittape* bit, const BITTAPE_INT val, const BITTAPE_LEN cnt)
{
  BITTAPE_INT idx;
  BITTAPE_INT off;

  // Compute the current writing word index.
  idx = bit->bt_put / BITTAPE_BIT;

  // Compute the offset within the word.
  off = bit->bt_put % BITTAPE_BIT;

  // Write the requested bits.
  bit->bt_buf[idx] |= ((BITTAPE_INT)val << off);

  // Advance the writing head.
  bit->bt_put += cnt;
}

/// Write bits to the tape at the writing head position.
/// @return success/failure indication
///
/// @param[in] bit bit tape
/// @param[in] len number of bits to write
/// @param[in] val bits to write 
bool
bittape_put(struct bittape* bit, const BITTAPE_LEN cnt, const BITTAPE_INT val)
{
  BITTAPE_INT spl; // Split index between the two words.
  BITTAPE_INT pos; // Bit index within the current word.
  BITTAPE_INT adj; // Adjusted bit index after the write.

  // Ensure that the tape has sufficient space for the requested bits.
  if (bit->bt_put + cnt > bit->bt_max) {
    return false;
  }

  // Determine where the split ought to be.
  pos =  bit->bt_put        % BITTAPE_BIT;
  adj = (bit->bt_put + cnt) % BITTAPE_BIT;
  spl = adj * (adj <= pos);

  // Perform the first write which always occurs.
  put(bit, val, cnt - spl);

  // Only perform the second write if any bits fall onto the second word.
  if (spl > 0) {
    put(bit, val >> (cnt - spl), spl);
  }

  return true;
}

/// Perform a read within a single word.
/// @return value
///
/// @param[in] bit bit tape
/// @param[in] cnt number of bits
static BITTAPE_INT 
get(struct bittape* bit, const BITTAPE_LEN cnt)
{
  BITTAPE_INT idx;
  BITTAPE_INT off;
  BITTAPE_INT val;

  // Compute the current reading word index.
  idx = bit->bt_get / BITTAPE_BIT;

  // Compute the offset within the word.
  off = bit->bt_get % BITTAPE_BIT;

  // Retrieve the value at the selected offset.
  val = bit->bt_buf[idx] >> off;

  // Remove all unwanted higher bits.
  if (cnt < BITTAPE_BIT) {
    val &= ((BITTAPE_INT)1 << cnt) - 1;
  }

  // Advance the reading head.
  bit->bt_get += cnt;

  return val;
}

/// Read bits from the tape.
/// @return success/failure
///
/// @param[in]  bit bit tape
/// @param[in]  cnt number of bits to read
/// @param[out] val read bits
bool
bittape_get(struct bittape* bit, const BITTAPE_LEN cnt, BITTAPE_INT* val)
{
  BITTAPE_INT spl;
  BITTAPE_INT pos;
  BITTAPE_INT adj;

  // Ensure that the tape has requested number of bits available.
  if (bit->bt_get + cnt > bit->bt_put) {
    return false;
  }

  // Determine where the split ought to be.
  pos =  bit->bt_get        % BITTAPE_BIT;
  adj = (bit->bt_get + cnt) % BITTAPE_BIT;
  spl = adj * (adj <= pos);

  // Perform two reads, one for each part.
  *val = get(bit, cnt - spl);

  // Only perform the second read if the request spans two words.
  if (spl > 0) {
    *val |= (get(bit, spl) << (cnt - spl));
  }

  return true;
}
