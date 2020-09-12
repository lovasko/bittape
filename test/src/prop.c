// Copyright (c) 2020 Daniel Lovasko
// All Rights Reserved
//
// Distributed under the terms of the 2-clause BSD License. The full
// license is in the file LICENSE, distributed as part of this software.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <inttypes.h>

#define BITTAPE_BUF
#include "bittape.h"


// Number of random tests to run.
#ifndef TEST_REP
  #define TEST_REP 100000
#endif

// Number of bit blocks written to the tape during each test.
#ifndef TEST_LEN
  #define TEST_LEN 10
#endif

/// Generate the full values of which that are to be written to the tape.
///
/// @param[out] val values
static void
gen_val(BITTAPE_WORD* val)
{
  uintmax_t idx;
  uintmax_t jdx;
  uint8_t   oct[sizeof(BITTAPE_WORD)];

  for (idx = 0; idx < TEST_LEN; idx += 1) {
    // Generate random bytes.
    for (jdx = 0; jdx < sizeof(BITTAPE_WORD); jdx += 1) {
      oct[jdx] = rand() % 256;
    }

    // Write the bytes to the appropriate integer.
    (void)memcpy(&val[idx], oct, sizeof(oct));
  }
}

/// Generate random bit counts and amend the values accordingly.
///
/// @param[out] cnt bit counts
/// @param[out] val values
static void
gen_cnt(BITTAPE_LEN *restrict cnt, BITTAPE_WORD *restrict val)
{
  uintmax_t idx;

  for (idx = 0; idx < TEST_LEN; idx += 1) {
    // Random number of bits between 0 and the size of BITTAPE_WORD in bits.
    cnt[idx] = (rand() % (sizeof(BITTAPE_WORD) * CHAR_BIT)) + 1;

    // Cull the higher bits of the respective value accordingly.
    val[idx] &= ((BITTAPE_WORD)1 << cnt[idx]) - 1;
  }
}

/// Generate the random order of reads and writes to the tape.
/// 
/// Throughout the program the following mapping is assumed:
///   * read  = 0 = false
///   * write = 1 = true
///
/// @param[out] ord order of reads and writes
static void
gen_ops(bool* ops)
{
  uintmax_t pos[2]; // Write and read count.
  uint8_t   rnd;    // Random number.

  // Start with both heads at the start.
  pos[0] = 0;
  pos[1] = 0;

  while (true) {
    // Check if all operations were assigned.
    if (pos[0] == TEST_LEN && pos[1] == TEST_LEN) {
      break;
    }

    // Roll a four-sided dice.
    rnd = rand() % 3;

    // The 1/4 probability of a read, but only if a sufficient number of writes happened.
    if (rnd == 0 && pos[0] < pos[1] && pos[0] < TEST_LEN) {
      ops[pos[0] + pos[1]] = false;
      pos[0] += 1;
      continue;
    }

    // Given the 3/4 probability and the availability of writes
    if (rnd > 0 && pos[1] < TEST_LEN) {
      ops[pos[0] + pos[1]] = true;
      pos[1] += 1;
    }
  }
}

/// Print the 
static void
unit(const BITTAPE_WORD *restrict val, const BITTAPE_LEN *restrict cnt, const bool* ops)
{
  uintmax_t put;
  uintmax_t get;
  uintmax_t idx;

  (void)printf("./lunit%u ", BITTAPE_BIT);

  put = 0;
  get = 0;
  for (idx = 0; idx < TEST_LEN * 2; idx += 1) {
    if (ops[idx] == true) {
      (void)printf("-p %" PRIuMAX ":%" PRIuMAX " ", (uintmax_t)cnt[put], (uintmax_t)val[put]);
      put += 1;
    } else {
      (void)printf("-g %" PRIuMAX ":%" PRIuMAX " ", (uintmax_t)cnt[get], (uintmax_t)val[get]);
      get += 1;
    }
  }

  (void)printf("\n");
}	

/// Run a single instance of the test.
/// @return success/failure indication
///
/// @param[in] bit bit tape
/// @param[in] val values
/// @param[in] cnt bit counts
/// @param[in] ops operations
static bool 
run(struct bittape *restrict bit, BITTAPE_WORD *restrict val, BITTAPE_LEN *restrict cnt, bool* ops)
{
  uintmax_t   pos[2];
  uintmax_t   idx;
  BITTAPE_WORD get;
  bool        ret;

  // Generate test data.
  gen_val(val);
  gen_cnt(cnt, val);
  gen_ops(ops);

  // Initialise the bit tape.
  (void)memset(bit, 0, sizeof(*bit));
  (void)memset(bit->bt_buf, 0, TEST_LEN * sizeof(BITTAPE_WORD));
  bittape_new(bit, TEST_LEN * sizeof(BITTAPE_WORD) * CHAR_BIT);

  // Both reading and writing positions start at the beginning.
  pos[0] = 0;
  pos[1] = 0;

  // Walk through the operations and verify read results.
  for (idx = 0; idx < TEST_LEN * 2; idx += 1) {
    // Perform a write.
    if (ops[idx] == true) {
      // Write bits to the tape.
      ret = bittape_put(bit, cnt[pos[1]], val[pos[1]]);
      if (ret == false) {
        printf("put failed\n");
        return false;
      }

      // Advance the writing position.
      pos[1] += 1;

    // Perform a read and verify its correctness.
    } else {
      // Read bits off the tape.
      ret = bittape_get(bit, cnt[pos[0]], &get);
      if (ret == false) {
        printf("get failed\n");
        return false;
      }

      // Ensure the data is the same as the ones written.
      if (get != val[pos[0]]) {
        (void)printf("mismatch: %" PRIuMAX " %" PRIuMAX "\n",
          (uintmax_t)get, (uintmax_t)val[pos[0]]);
	unit(val, cnt, ops);
        return false;
      }

      // Advance the reading position.
      pos[0] += 1;
    }
  }

  return true;
}

int
main(void)
{
  struct bittape* bit;
  BITTAPE_WORD*   val;
  BITTAPE_LEN*    cnt;
  bool*           ops;
  uintmax_t       rep;
  time_t          now;
  bool            ret;

  // Allocate memory for the bit tape.
  bit = malloc(sizeof(struct bittape) + (sizeof(BITTAPE_WORD) * TEST_LEN));

  // Allocate memory for the testing data.
  val = malloc(sizeof(*val) * TEST_LEN);
  cnt = malloc(sizeof(*cnt) * TEST_LEN);
  ops = malloc(sizeof(*ops) * TEST_LEN * 2);

  // Run the random test repeatedly.
  now = time(NULL);
  srand(now);
  ret = true;
  for (rep = 0; rep < TEST_REP; rep += 1) {
    ret = run(bit, val, cnt, ops);
    if (ret == false) {
      break;
    }
  }

  // Deallocate all used memory.
  free(bit);
  free(val);
  free(cnt);
  free(ops);

  // Select the appropriate return code.
  if (ret == true) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
