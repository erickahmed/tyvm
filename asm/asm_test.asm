.ORIG x3000
  AND R0, R0, #0
  AND R1, R1, #0
  AND R3, R3, #0
  LEA R0, NUM
  ADD R1, R1, R0
  LD R2, ASCII

FOR_LOOP
  LDR R4, R1, #0
  BRz END_LOOP
  ADD R4, R4, R2
  STR R4, R1, #0
  ADD R1, R1, #1
  BRnzp FOR_LOOP
END_LOOP

  PUTs
HALT

ASCII .fill  x30
NUM   .fill  x01
      .fill  x02
      .fill  x03
      .fill  x04
.END