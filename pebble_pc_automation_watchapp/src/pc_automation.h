#pragma once

#include <pebble.h>

#define DEBUG 0

#if DEBUG
#define NUM_FIRST_MENU_ITEMS 8
#else
#define NUM_FIRST_MENU_ITEMS 7
#endif

void app_message_init();
int get_last_row_clicked();
void update_row_subtitle(int row_num, const char *subtitle);
void run_pc_script(int row_num);
void get_server_addr();

