#include "ttom.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static int remove_nl(char *str)
{
  int endpt;

  endpt = -1;
  while (str[++endpt]);
  str[--endpt] = '\0';

  return 0;
}

/*
 * MDLD_ttom
 *
 * TODO: Variables are a mess
 *
 * @desc
 *   Takes a .mdld.txt and creates
 *   an equivalent .mdld file
 * @param fpath
 *   Path to the .mdld.txt file
 */
#define READ_LINE(block) \
  if (!fgets(temp, 256, in)) { \
    if (ferror(in)) { \
      printf("Error reading file (fgets)\n"); \
      return -1; \
    } \
    if (feof(in)) { \
      printf("Reached EOF before " #block "_END\n"); \
      return -1; \
    } \
  } \
  remove_nl(temp);
int MDLD_ttom(const char *fpath)
{
  FILE    *in;
  FILE    *out;
  int      endpt;
  char     temp[256];
  char     temp2[256]; /* For processing vertex values */
  long int temp2_pos;
  int32_t  count;
  int32_t  temp_int;
  long int pos;
  float    vrtx;

  if (!(in = fopen(fpath, "r"))) { printf("Could not open file: %s\n", fpath); return -1; }
  endpt = -1;
  while (fpath[++endpt]);
  endpt -= 4;
  strncat(temp, fpath, endpt);
  if (!(out = fopen(temp, "wb"))) { printf("Could not open file: %s\n", temp); return -1; }

  while (fgets(temp, 256, in)) {
    remove_nl(temp);
    if (!strcmp(temp, "DATA_START")) {
      count = 0;
      if (fwrite(&count, sizeof(int32_t), 1, out) != 1) { printf("Could not write\n"); return -1; }
      while (strcmp(temp, "DATA_END")) {
        READ_LINE(DATA);

        if (!strcmp(temp, "DATA_END")) continue;
        endpt = 0;
        temp2_pos = -1;
        while (temp[endpt]) {
          if (temp[endpt] == ',') {
            temp2[++temp2_pos] = '\0';
            vrtx = (float) atof(temp2);
            if (fwrite(&vrtx, sizeof(float), 1, out) != 1) { printf("Could not write vertex\n"); return -1; }
            ++count;
            temp2_pos = -1;
          }
          else {
            temp2[++temp2_pos] = temp[endpt];
          }
          ++endpt;
        }

        if ((pos = ftell(out)) == -1) { printf("ftell failed\n"); return -1; }
        if (fseek(out, 0, SEEK_SET)) { printf("Seek failed"); return -1; }
        if (fwrite(&count, sizeof(int32_t), 1, out) != 1) { printf("Could not write\n"); return -1; }
        if (fseek(out, pos, SEEK_SET)) { printf("Seek failed"); return -1; }
      }
    }
    else if (!strcmp(temp, "FORMAT_START")) {
      count = 0;
      long int start_pos = ftell(out);
      if (fwrite(&count, sizeof(int32_t), 1, out) != 1) { printf("Could not write\n"); return -1; }
      while (strcmp(temp, "FORMAT_END")) {
        READ_LINE(FORMAT);
        if (!strcmp(temp, "FORMAT_END")) continue;
        ++count;

        endpt = 0;
        temp2_pos = -1;
        while (temp[endpt]) {
          if (temp[endpt] == ',') {
            temp2[++temp2_pos] = '\0';
            temp_int = (int32_t) atoi(temp2);
            if (fwrite(&temp_int, sizeof(int32_t), 1, out) != 1) { printf("Could not write format\n"); return -1; }
            temp2_pos = -1;
          }
          else {
            temp2[++temp2_pos] = temp[endpt];
          }
          ++endpt;
        }
        temp2[++temp2_pos] = '\0';
        temp_int = (int32_t) atoi(temp2);
        if (fwrite(&temp_int, sizeof(int32_t), 1, out) != 1) { printf("Could not write format\n"); return -1; }

        if ((pos = ftell(out)) == -1) { printf("ftell failed\n"); return -1; }
        if (fseek(out, start_pos, SEEK_SET)) { printf("Seek failed"); return -1; }
        if (fwrite(&count, sizeof(int32_t), 1, out) != 1) { printf("Could not write\n"); return -1; }
        if (fseek(out, pos, SEEK_SET)) { printf("Seek failed"); return -1; }
      }
    }
    else if (!strcmp(temp, "COMMENT_START")) {
      while (strcmp(temp, "COMMENT_END")) {
        READ_LINE(COMMENT);
      }
    }
    else {
      printf("Unrecognized block\n");
    }
  }
  if (ferror(in)) {
    printf("Error reading file (fgets)\n");
    return -1;
  }

  fclose(in);
  fclose(out);

  return 0;
}
