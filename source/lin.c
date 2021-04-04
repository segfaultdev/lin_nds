#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <time.h>

#include "cga8.h"
#include "data.h"

#include "lin.h"

#define TILE_X(x) ((x) * 3 + 2)
#define TILE_Y(y) ((((y) * 3) + 1) * 32)
#define TILE_TO_IDX(x, y) (TILE_X(x) + TILE_Y(y))

#define VIRT_KEY_SAVE  ((char)(1))
#define VIRT_KEY_OPEN  ((char)(2))
#define VIRT_KEY_COPY  ((char)(3))
#define VIRT_KEY_PASTE ((char)(4))
#define VIRT_KEY_CUT   ((char)(5))
#define VIRT_KEY_HELP  ((char)(6))
#define VIRT_KEY_EXIT  ((char)(7))

int top_bg, sub_bg;

uint16_t *cursor_gfx;

void setup_screens(void) {
  videoSetMode(MODE_0_2D);
  vramSetBankA(VRAM_A_MAIN_BG);
  vramSetBankB(VRAM_B_MAIN_SPRITE);

  videoSetModeSub(MODE_0_2D);
  vramSetBankC(VRAM_C_SUB_BG);

  top_bg = bgInit(0, BgType_Text4bpp, BgSize_T_256x256, 0, 1);
  sub_bg = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 0, 1);

  bgSetPriority(top_bg, 3);

  oamInit(&oamMain, SpriteMapping_1D_32, false);

  cursor_gfx = oamAllocateGfx(&oamMain, SpriteSize_8x8, SpriteColorFormat_16Color);

  for (int i = 0; i < 16; i++) {
    BG_PALETTE[(i << 4) +  0] = BG_PALETTE_SUB[(i << 4) +  0] = SPRITE_PALETTE[(i << 4) +  0] = RGB15(0, 0, 0);
    BG_PALETTE[(i << 4) + 15] = BG_PALETTE_SUB[(i << 4) + 15] = SPRITE_PALETTE[(i << 4) + 15] = palette[i];
  }

  dmaCopy(font, (void *)(bgGetGfxPtr(top_bg)), sizeof(font));
  dmaCopy(font, (void *)(bgGetGfxPtr(sub_bg)), sizeof(font));

  memset(bgGetMapPtr(top_bg), 0, 4096);

  for (int i = 0; i < 16; i++) cursor_gfx[i] = -(i >= 14);
}

int update_keyboard(int tile_x, int tile_y, int shift) {
  int key = 0;

  const char *layout = (const char *)(shift ? key_layout_shift_str : key_layout_str);

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 10; j++) {
      int is_extra = layout[j + i * 11] < 32 || layout[j + i * 11] >= 128;

      int idx = TILE_TO_IDX(j, i + 1);
      int color = is_extra ? 15 : 7;

      if (tile_x >= ((j * 3) + 1) && tile_x <= ((j * 3) + 3) && tile_y >= ((i * 3) + 3) && tile_y <= ((i * 3) + 5)) {
        color = 9;
        key = layout[j + i * 11];
      }

      if (layout[j + i * 11] == '\030' && shift) color = 9;

      bgGetMapPtr(sub_bg)[idx] = (color << 12) | (1 << 8) | layout[j + i * 11];
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 28; j++) {
      bgGetMapPtr(sub_bg)[j + 2 + ((i * 3) + 19) * 32] = (15 << 12) | (1 << 8) | bottom_str[i][j];
    }
  }

  for (int i = 0; path_str[i]; i++) {
    bgGetMapPtr(sub_bg)[i + 34] = (15 << 12) | (1 << 8) | path_str[i];
  }

  if (tile_y >= 18 && tile_y <= 20) {
    if (tile_x >= 1 && tile_x <= 6) {
      key = VIRT_KEY_SAVE;

      for (int i = 2; i <= 5; i++) bgGetMapPtr(sub_bg)[i + TILE_Y(6)] = (9 << 12) | (1 << 8) | bottom_str[0][i - 2];
    } else if (tile_x >= 7 && tile_x <= 24) {
      key = ' ';

      for (int i = 8; i <= 23; i++) bgGetMapPtr(sub_bg)[i + TILE_Y(6)] = (9 << 12) | (1 << 8) | bottom_str[0][i - 2];
    } else if (tile_x >= 25 && tile_x <= 30) {
      key = VIRT_KEY_OPEN;

      for (int i = 26; i <= 29; i++) bgGetMapPtr(sub_bg)[i + TILE_Y(6)] = (9 << 12) | (1 << 8) | bottom_str[0][i - 2];
    }
  } else if (tile_y >= 21 && tile_y <= 23) {
    if (tile_x >= 1 && tile_x <= 6) {
      key = VIRT_KEY_COPY;

      for (int i = 2; i <= 5; i++) bgGetMapPtr(sub_bg)[i + TILE_Y(7)] = (9 << 12) | (1 << 8) | bottom_str[1][i - 2];
    } else if (tile_x >= 7 && tile_x <= 13) {
      key = VIRT_KEY_PASTE;

      for (int i = 8; i <= 12; i++) bgGetMapPtr(sub_bg)[i + TILE_Y(7)] = (9 << 12) | (1 << 8) | bottom_str[1][i - 2];
    } else if (tile_x >= 14 && tile_x <= 18) {
      key = VIRT_KEY_CUT;

      for (int i = 15; i <= 17; i++) bgGetMapPtr(sub_bg)[i + TILE_Y(7)] = (9 << 12) | (1 << 8) | bottom_str[1][i - 2];
    } else if (tile_x >= 19 && tile_x <= 24) {
      key = VIRT_KEY_HELP;

      for (int i = 20; i <= 23; i++) bgGetMapPtr(sub_bg)[i + TILE_Y(7)] = (9 << 12) | (1 << 8) | bottom_str[1][i - 2];
    } else if (tile_x >= 25 && tile_x <= 30) {
      key = VIRT_KEY_EXIT;

      for (int i = 26; i <= 29; i++) bgGetMapPtr(sub_bg)[i + TILE_Y(7)] = (9 << 12) | (1 << 8) | bottom_str[1][i - 2];
    }
  }

  return key;
}

void render_screen(lin_t *file, int *x, int *y) {
  int curr_x = 0, curr_y = 0;
  int set_xy = 0;

  for (int i = 0; i < file->size; i++) {
    char c = file->buff[i];

    int inv = i >= file->start_pos && i < file->end_pos;

    if (i == file->end_pos) {
      *x = curr_x, *y = curr_y;
      set_xy = 1;
    }

    if (c == '\n') {
      curr_x = 0;
      curr_y++;
    } else if (c >= ' ') {
      bgGetMapPtr(top_bg)[curr_x + (curr_y << 5)] = (7 << 12) | (inv << 9) | c;
      curr_x++;
    }
  }

  if (!set_xy) *x = curr_x, *y = curr_y;
}

int main(void) {
  // if (!fatInitDefault()) return 1;
  consoleDebugInit(DebugDevice_NOCASH);
  setup_screens();

  timerStart(0, ClockDivider_1024, 0, NULL);
  int ticks = 0;

  int keys_held, keys_down, keys_up;

  int virt_key = 0, shift = 0;

  touchPosition touch;

  lin_t file;
  lin_init(&file);

  keysSetRepeat(90, 5);

  fprintf(stderr, "Hey!\n");

  while(1) {
    swiWaitForVBlank();

    scanKeys();
    keys_held = keysHeld();
    keys_down = keysDownRepeat();
    keys_up = keysUp();

    int tile_x = -1, tile_y = -1;
    int pos_x, pos_y;

    if (keys_held & KEY_TOUCH) {
      touchRead(&touch);

      tile_x = touch.px >> 3;
      tile_y = touch.py >> 3;
    }

    if (keys_down & KEY_RIGHT) {
      lin_move_right(&file, shift);
    } else if (keys_down & KEY_LEFT) {
      lin_move_left(&file, shift);
    } else if (keys_down & KEY_DOWN) {
      lin_move_down(&file, shift);
    } else if (keys_down & KEY_UP) {
      lin_move_up(&file, shift);
    } else if (keys_down & KEY_A) {
      lin_putchar(&file, '\n');
    } else if (keys_down & KEY_B) {
      lin_delchar(&file);
    }

    if (keys_up & KEY_TOUCH && virt_key) {
      ticks = 0;

      if (virt_key == '\030') {
        shift = !shift;
      } else if (virt_key == VIRT_KEY_EXIT) {
        break;
      } else if (virt_key >= ' ') {
        lin_putchar(&file, virt_key);
      }
    }

    virt_key = update_keyboard(tile_x, tile_y, shift);

    render_screen(&file, &pos_x, &pos_y);

    oamSet(&oamMain, 0, pos_x << 3, pos_y << 3, 0, 15, SpriteSize_8x8, SpriteColorFormat_16Color, cursor_gfx, -1, 0, ((ticks * 2048) / BUS_CLOCK) & 1, 0, 0, 0);
    oamUpdate(&oamMain);

    ticks += timerElapsed(0);
  }

  timerStop(0);

  return 0;
}
