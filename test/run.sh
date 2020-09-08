#!/bin/sh
# Copyright (c) 2020 Daniel Lovasko
# All Rights Reserved
#
# Distributed under the terms of the 2-clause BSD License. The full
# license is in the file LICENSE, distributed as part of this software.

# Compilation settings.
C99="cc"
STD="-std=c99"
FLG="-Wall -Wextra -Werror -D_XOPEN_SOURCE"
OPT="-O0"
INC="-I../src/"

# Ensure that any failure fails the whole testing process.
set -e

# Prepare the directory for storing testing programs.
mkdir -p bin

# Compile the property tests in four bit widths.
${C99} ${STD} ${FLG} ${OPT} ${INC} -DTEST_LEN=16    -DBITTAPE_BIT=8  \
  -o bin/prop8   src/prop.c ../src/bittape.c

${C99} ${STD} ${FLG} ${OPT} ${INC} -DTEST_LEN=16    -DBITTAPE_BIT=16 \
  -o bin/prop16  src/prop.c ../src/bittape.c

${C99} ${STD} ${FLG} ${OPT} ${INC} -DTEST_LEN=16    -DBITTAPE_BIT=32 \
  -o bin/prop32  src/prop.c ../src/bittape.c

${C99} ${STD} ${FLG} ${OPT} ${INC} -DTEST_LEN=16    -DBITTAPE_BIT=64 \
  -o bin/prop64  src/prop.c ../src/bittape.c

# Compile and unit test program for small inputs.
${C99} ${STD} ${FLG} ${OPT} ${INC} -DBITTAPE_BUF=2  -DBITTAPE_BIT=8  \
  -o bin/sunit8  src/unit.c ../src/bittape.c

${C99} ${STD} ${FLG} ${OPT} ${INC} -DBITTAPE_BUF=2  -DBITTAPE_BIT=16 \
  -o bin/sunit16 src/unit.c ../src/bittape.c

${C99} ${STD} ${FLG} ${OPT} ${INC} -DBITTAPE_BUF=2  -DBITTAPE_BIT=32 \
  -o bin/sunit32 src/unit.c ../src/bittape.c

${C99} ${STD} ${FLG} ${OPT} ${INC} -DBITTAPE_BUF=2  -DBITTAPE_BIT=64 \
  -o bin/sunit64 src/unit.c ../src/bittape.c

# Compile the unit test program for large inputs.
${C99} ${STD} ${FLG} ${OPT} ${INC} -DBITTAPE_BUF=16 -DBITTAPE_BIT=8  \
  -o bin/lunit8  src/unit.c ../src/bittape.c

${C99} ${STD} ${FLG} ${OPT} ${INC} -DBITTAPE_BUF=16 -DBITTAPE_BIT=16 \
  -o bin/lunit16 src/unit.c ../src/bittape.c

${C99} ${STD} ${FLG} ${OPT} ${INC} -DBITTAPE_BUF=16 -DBITTAPE_BIT=32 \
  -o bin/lunit32 src/unit.c ../src/bittape.c

${C99} ${STD} ${FLG} ${OPT} ${INC} -DBITTAPE_BUF=16 -DBITTAPE_BIT=64 \
  -o bin/lunit64 src/unit.c ../src/bittape.c

# Ensure that each test case also has its definition visible.
set -x

# Run the 8-bit unit tests.
bin/sunit8 -G1
bin/sunit8 -G2       -G3
bin/sunit8 -p1:1     -g1:1
bin/sunit8 -p1:1     -g1:1    -G1
bin/sunit8 -p2:3     -p3:2    -g2:3   -g3:2
bin/sunit8 -p2:3     -g2:3    -p3:2   -g3:2
bin/sunit8 -p5:21    -p5:10   -g5:21  -g5:10
bin/sunit8 -p8:255   -p8:255  -P1
bin/sunit8 -p3:2     -G5      -G4     -g3:2

# Run the 16-bit unit tests.
bin/sunit16 -G1
bin/sunit16 -G2      -G3
bin/sunit16 -p1:1    -g1:1
bin/sunit16 -p1:1    -g1:1    -G1
bin/sunit16 -p2:3    -p3:2    -g2:3   -g3:2
bin/sunit16 -p2:3    -g2:3    -p3:2   -g3:2
bin/sunit16 -p9:341  -p9:170  -g9:341 -g9:170
bin/sunit16 -p16:255 -p16:255 -P1
bin/sunit16 -p3:2    -G5      -G4     -g3:2

# Run the 32-bit unit tests.
bin/sunit32 -G1
bin/sunit32 -G2      -G3
bin/sunit32 -p1:1    -g1:1
bin/sunit32 -p1:1    -g1:1    -G1
bin/sunit32 -p2:3    -p3:2    -g2:3    -g3:2
bin/sunit32 -p2:3    -g2:3    -p3:2    -g3:2
bin/sunit32 -p21:682 -p21:340 -g21:682 -g21:340
bin/sunit32 -p32:255 -p32:255 -P1
bin/sunit32 -p3:2    -G5      -G4      -g3:2

# Run the 64-bit unit tests.
bin/sunit64 -G1
bin/sunit64 -G2      -G3
bin/sunit64 -p1:1    -g1:1
bin/sunit64 -p1:1    -g1:1    -G1
bin/sunit64 -p2:3    -p3:2    -g2:3    -g3:2
bin/sunit64 -p2:3    -g2:3    -p3:2    -g3:2
bin/sunit64 -p43:682 -p43:340 -g43:682 -g43:340
bin/sunit64 -p64:255 -p64:255 -P1
bin/sunit64 -p3:2    -G5      -G4      -g3:2

# No need to report runs of the property tests.
set +x

# Run the property tests.
bin/prop8
bin/prop16
bin/prop32
bin/prop64
