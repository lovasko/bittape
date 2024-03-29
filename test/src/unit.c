// Copyright (c) 2020 Daniel Lovasko
// All Rights Reserved
//
// Distributed under the terms of the 2-clause BSD License. The full
// license is in the file LICENSE, distributed as part of this software.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <limits.h>

#include "bittape.h"


/// Perform a take read that ought to succeed.
/// @return success/failure indication
///
/// @param[in] tape bit tape
/// @param[in] cnt  number of bits
/// @param[in] exp  expected bits
static bool
succ_get(struct bittape* tape, const BITTAPE_LEN cnt, const BITTAPE_WORD exp)
{
  BITTAPE_WORD act;
  bool         ret;

  // Perform the read.
  ret = bittape_get(tape, cnt, &act);
  if (ret == false) {
    (void)printf("get failed\n");
    return false;
  }

  // Verify that the expected and actual bits match.
  if (exp != act) {
    (void)printf(
      "get(%" PRIuMAX ") value mismatch: "
      "exp=%" PRIuMAX " act=%" PRIuMAX "\n",
      (uintmax_t)cnt, (uintmax_t)exp, (uintmax_t)act);
    return false;
  }

  return true;
}

/// Perform a tape read that ought to fail.
/// @return success/failure indication
///
/// @param[in] tape bit tape
/// @param[in] cnt  number of bits
static bool
fail_get(struct bittape* tape, const BITTAPE_LEN cnt)
{
  BITTAPE_WORD val;
  bool         ret;

  ret = bittape_get(tape, cnt, &val);
  if (ret == true) {
    (void)printf("get succeeded but was supposed to fail\n");
    return false;
  }

  return true;
}

/// Perform a tape write that ought to succeed.
/// @return success/failure indication
///
/// @param[in] tape bit tape
/// @param[in] cnt  number of bits
/// @param[in] val  bits
static bool
succ_put(struct bittape* tape, const BITTAPE_LEN cnt, const BITTAPE_WORD val)
{
  bool ret;

  ret = bittape_put(tape, cnt, val);
  if (ret == false) {
    (void)printf("put failed\n");
    return false;
  }

  return true;
}

/// Perform a take write that ought to fail.
/// @return success/failure indication
///
/// @param[in] tape bit tape
/// @param[in] cnt  number of bits
static bool
fail_put(struct bittape* tape, const BITTAPE_LEN cnt)
{
  BITTAPE_WORD val;
  bool         ret;

  val = 0;
  ret = bittape_put(tape, cnt, val);
  if (ret == true) {
    (void)printf("put succeeded but was supposed to fail\n");
    return false;
  }

  return true;
}

/// Pretty-print the contents of the tape.
///
/// @param[in] tape bit tape
static void
tape_str(const struct bittape* tape)
{
  uintmax_t idx;
  uintmax_t jdx;
  uintmax_t kdx;
  char      str[CHAR_BIT];

  for (idx = 0; idx < BITTAPE_BUF; idx += 1) {
    for (jdx = 0; jdx < sizeof(BITTAPE_WORD); jdx += 1) {
      // Convert a byte to a string.
      for (kdx = 0; kdx < CHAR_BIT; kdx += 1) {
        str[kdx] = tape->bt_buf[idx] & ((BITTAPE_WORD)1 << (jdx * CHAR_BIT + kdx)) ? '1' : '0';
      }

      // Print the string with a delimiter in the middle.
      (void)printf("%c%c%c%c-%c%c%c%c", str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);

      // Connect the bytes within a word with an underscore.
      if (jdx != (sizeof(BITTAPE_WORD) - 1)) {
        (void)printf("_");
      }
    }

    // Separate words with a multiple spaces.
    if (idx != (BITTAPE_BUF - 1)) {
      (void)printf("   ");
    }
  }

  (void)printf("\n");
}

/// Run a set of commands from the command-line arguments and report issues.
int
main(int argc, char* argv[])
{
  struct bittape tape;
  uintmax_t      val;
  uintmax_t      cnt;
  int            mtch;
  int            opt;
  bool           ret;

  // Initialise the tape.
  (void)memset(&tape, 0, sizeof(tape));
  bittape_new(&tape, BITTAPE_BUF * BITTAPE_BIT);
  tape_str(&tape);

  while (true) {
    // Read the next command.
    opt = getopt(argc, argv, "p:g:P:G:");
    if (opt == -1) {
      return EXIT_SUCCESS;
    }

    // Parse the command parameters.
    if (opt == 'p' || opt == 'g') {
      mtch = sscanf(optarg, "%" SCNuMAX ":%" SCNuMAX, &cnt, &val);
      if (mtch != 2) {
        (void)printf("unable to parse the command parameters: '%s'\n", optarg);
        return EXIT_FAILURE;
      }
    }

    if (opt == 'P' || opt == 'G') {
      mtch = sscanf(optarg, "%" SCNuMAX, &cnt);
      if (mtch != 1) {
        (void)printf("unable to parse the command parameter: '%s'\n", optarg);
        return EXIT_FAILURE;
      }
    }

    // Execute the appropriate command.
    if (opt == 'p') {
      ret = succ_put(&tape, (BITTAPE_LEN)cnt, (BITTAPE_WORD)val);
      if (ret == false) {
        return EXIT_FAILURE;
      }
    }

    if (opt == 'g') {
      ret = succ_get(&tape, (BITTAPE_LEN)cnt, (BITTAPE_WORD)val);
      if (ret == false) {
        return EXIT_FAILURE;
      }
    }

    if (opt == 'P') {
      ret = fail_put(&tape, (BITTAPE_LEN)cnt);
      if (ret == false) {
        return EXIT_FAILURE;
      }
    }

    if (opt == 'G') {
      ret = fail_get(&tape, (BITTAPE_LEN)cnt);
      if (ret == false) {
        return EXIT_FAILURE;
      }
    }

    // Print the tape if a successful write was performed.
    if (opt == 'p') {
      tape_str(&tape);
    }
  }

  return EXIT_SUCCESS;
}
