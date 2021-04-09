#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[100];
  uint32_t new_val;
  uint32_t old_val;
  uint32_t temp;
  /* TODO: Add more members if necessary */


} WP;
WP* new_wp();
void free_wp(WP *wp);
int set_watchpoint(char *e);
bool delete_watchpoint(int NO);
void list_watchpoint(void);
WP* scan_watchpoint(void);
uint32_t getbyte(uint32_t addr);
uint32_t addr;
#endif

