#ifndef __DATA_H
#define __DATA_H

#include <stdint.h>

const uint16_t palette[] = {
  0x0000, 0x000F, 0x01E0, 0x01EF,
  0x3C00, 0x3C0F, 0x3DE0, 0x3DEF,

  0x1CE7, 0x1CFF, 0x1FE7, 0x1FFF,
  0x7CE7, 0x7CFF, 0x7FE7, 0x7FFF
};

const char key_layout_str[][11] = {
  "1234567890",
  "qwertyuiop",
  "\035asdfghjkl",
  "\030zxcvbnm,.",
  "\373()-/\\'[]+"
};

const char key_layout_shift_str[][11] = {
  "!@#$%^&*~`",
  "QWERTYUIOP",
  "\035ASDFGHJKL",
  "\030ZXCVBNM;:",
  "\373{}_?|\"<>="
};

const char bottom_str[][29] = {
  "Save  ________________  Open",
  "Copy  Paste  Cut  Help  Exit"
};

const char path_str[] = "Path: ";

#endif
