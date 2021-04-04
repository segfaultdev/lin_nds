// All the editor's real code

#ifndef __LIN_H
#define __LIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

typedef struct lin_t lin_t;

struct lin_t {
  char path[256];

  char *buff;

  int size, start_pos, end_pos, cam_x, cam_y;
};

void lin_init(lin_t *file) {
  file->buff = NULL;

  file->size = file->start_pos = file->end_pos = file->cam_x = file->cam_y = 0;
}

int lin_open(lin_t *file, const char *path) {
  FILE *open_file = fopen(path, "r");
  if (!open_file) return 0;

  if (file->buff) free(file->buff);

  strcpy(file->path, path);

  fseek(open_file, 0, SEEK_END);
  file->size = ftell(open_file);
  fseek(open_file, 0, SEEK_SET);

  if (!(file->buff = malloc(file->size))) return 0;

  char c;
  int new_size = 0;

  for (int i = 0; i < file->size; i++) {
    if (!fread(&c, 1, 1, open_file)) return 0;

    if (c >= ' ' || c == '\n') {
      file->buff[new_size++] = c;
    }
  }

  file->size = new_size;
  file->start_pos = file->end_pos = 0;

  fclose(open_file);

  return 1;
}

int lin_line(lin_t *file) {
  int curr_y = 0;

  for (int i = 0; i < file->size; i++) {
    if (i == file->end_pos) break;

    if (file->buff[i] == '\n') curr_y++;
  }

  fprintf(stderr, "lin_line(): Curr. line: %d\n", curr_y);

  return curr_y;
}

int lin_length(lin_t *file, int line) {
  int curr_y = 0, len = 0;

  for (int i = 0; i < file->size; i++) {
    len++;

    if (file->buff[i] == '\n') {
      if (curr_y == line) break;

      len = 0;
      curr_y++;
    }
  }

  fprintf(stderr, "lin_length(): Length of %d: %d\n", line, len);

  return len;
}

void lin_limit(lin_t *file, int shift) {
  if (file->start_pos > file->end_pos || !shift) {
    file->start_pos = file->end_pos;
  }

  if (file->start_pos < 0) file->start_pos = 0;
  else if (file->start_pos > file->size) file->start_pos = file->size;

  if (file->end_pos < 0) file->end_pos = 0;
  else if (file->end_pos > file->size) file->end_pos = file->size;
}

void lin_move_right(lin_t *file, int shift) {
  file->end_pos++;

  lin_limit(file, shift);
}

void lin_move_left(lin_t *file, int shift) {
  file->end_pos--;

  lin_limit(file, shift);
}

void lin_move_down(lin_t *file, int shift) {
  int line = lin_line(file);

  if (line < 0) file->end_pos = file->size;
  else file->end_pos += lin_length(file, lin_line(file));

  lin_limit(file, shift);
}

void lin_move_up(lin_t *file, int shift) {
  int line = lin_line(file);

  if (line < 0) file->end_pos = 0;
  else file->end_pos -= lin_length(file, line);

  lin_limit(file, shift);
}

void lin_putchar(lin_t *file, char c) {
  char *new_buff = malloc(file->size + 1);
  int new_size = 0;

  for (int i = 0; i < file->size; i++) {
    if (new_size == file->start_pos) new_buff[new_size++] = c;
    if (!(i >= file->start_pos && i < file->end_pos)) new_buff[new_size++] = file->buff[i];
  }

  if (new_size == file->start_pos) new_buff[new_size++] = c;

  free(file->buff);

  fprintf(stderr, "lin_putchar(): Cursor: %d->%d, old size: %d, new size: %d\n", file->start_pos, file->end_pos, file->size, new_size);

  file->buff = new_buff;
  file->size = new_size;

  file->end_pos = file->start_pos;
  lin_move_right(file, false);
}

void lin_delchar(lin_t *file) {
  char *new_buff = malloc(file->size + 1);
  int new_size = 0;

  int block_del = file->start_pos != file->end_pos;

  for (int i = 0; i < file->size; i++) {
    if (block_del) {
      if (!(i >= file->start_pos && i < file->end_pos)) new_buff[new_size++] = file->buff[i];
    } else {
      if (i != (file->end_pos - 1)) new_buff[new_size++] = file->buff[i];
    }
  }

  free(file->buff);

  fprintf(stderr, "lin_delchar(): Cursor: %d->%d, old size: %d, new size: %d\n", file->start_pos, file->end_pos, file->size, new_size);

  file->buff = new_buff;
  file->size = new_size;

  file->end_pos = file->start_pos;
  if (!block_del) lin_move_left(file, false);
}

#endif
