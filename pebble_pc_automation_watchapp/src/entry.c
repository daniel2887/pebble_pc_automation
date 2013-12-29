#include <pebble.h>

#include "entry.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static struct EntryUi {
  Window *window;
  TextLayer *ip_title_layer;
  char ip_title_text[16];
  TextLayer *ip_digit_layers[12];
  char ip_digit_text[12][2];

  TextLayer *port_title_layer;
  char port_title_text[16];
  TextLayer *port_digit_layers[4];
  char port_digit_text[4][2];

  uint8_t section; /* 0 == IP, 1 = Port */
  uint8_t index;

  InverterLayer *invert_layer;
} ui;

char host_addr[32];
EntryCallback hs_callback;

GSize invert_layer_size = {
	.h = 31,
	.w = 12,
};


static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
	if (ui.section == 0) {
		ui.ip_digit_text[ui.index][0] = ui.ip_digit_text[ui.index][0] == '9' ? '0' :
			ui.ip_digit_text[ui.index][0] + 1;
		layer_mark_dirty(text_layer_get_layer(ui.ip_digit_layers[ui.index]));
	} else {
		ui.port_digit_text[ui.index][0] = ui.port_digit_text[ui.index][0] == '9' ? '0' :
			ui.port_digit_text[ui.index][0] + 1;
		layer_mark_dirty(text_layer_get_layer(ui.port_digit_layers[ui.index]));
	}
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
	if (ui.section == 0) {
		ui.ip_digit_text[ui.index][0] = ui.ip_digit_text[ui.index][0] == '0' ? '9' :
			ui.ip_digit_text[ui.index][0] - 1;
		layer_mark_dirty(text_layer_get_layer(ui.ip_digit_layers[ui.index]));
	} else {
		ui.port_digit_text[ui.index][0] = ui.port_digit_text[ui.index][0] == '0' ? '9' :
			ui.port_digit_text[ui.index][0] - 1;
		layer_mark_dirty(text_layer_get_layer(ui.ip_digit_layers[ui.index]));
	}
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
	Layer *window_layer = window_get_root_layer(ui.window);
	GRect bounds = layer_get_bounds(window_layer);
	int16_t invert_layer_h;

	inverter_layer_destroy(ui.invert_layer);

	if (ui.section == 0) {
		/* IP section */
		if (ui.index == ARRAY_SIZE(ui.ip_digit_text) - 1) {
			ui.index = 0;
			ui.section = 1;
			invert_layer_h = bounds.size.h * 3 / 4;
		} else {
			++ui.index;
			invert_layer_h = bounds.size.h / 4;
		}
	} else {
		/* Port section */
		if (ui.index == ARRAY_SIZE(ui.port_digit_text) - 1) {
			ui.index = 0;
			ui.section = 0;
			invert_layer_h = bounds.size.h / 4;
		} else {
			++ui.index;
			invert_layer_h = bounds.size.h * 3 / 4;
		}
	}

	ui.invert_layer = inverter_layer_create((GRect) {
			.origin = { 12 * ui.index + 1, invert_layer_h },
			.size = invert_layer_size
			});

	layer_add_child(window_get_root_layer(ui.window),
			inverter_layer_get_layer(ui.invert_layer));
}

static void click_config_provider(void *context) {
  const uint16_t repeat_interval_ms = 100;
  window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(ui.window);
  GRect bounds = layer_get_bounds(window_layer);

  /*
   * Set title layers
   */
 
  /* IP section */

  strncpy(ui.ip_title_text, "IP:", sizeof(ui.ip_title_text));
  ui.ip_title_layer = text_layer_create((GRect) {
        .origin = { 0, 0 },
        .size = { bounds.size.w, bounds.size.h / 4 }
      });
  text_layer_set_text(ui.ip_title_layer, ui.ip_title_text);
  text_layer_set_text_alignment(ui.ip_title_layer, GTextAlignmentLeft);
  text_layer_set_font(ui.ip_title_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(ui.ip_title_layer));

  /* Port section */

  strncpy(ui.port_title_text, "Port:", sizeof(ui.port_title_text));
  ui.port_title_layer = text_layer_create((GRect) {
        .origin = { 0, bounds.size.h / 2 },
        .size = { bounds.size.w, bounds.size.h * 3 / 4 }
      });
  text_layer_set_text(ui.port_title_layer, ui.port_title_text);
  text_layer_set_text_alignment(ui.port_title_layer, GTextAlignmentLeft);
  text_layer_set_font(ui.port_title_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(ui.port_title_layer));
 
  /* 
   * Set input fields layers and initial text
   */

  ui.index = 0;

  /* IP section */

  for (int i = 0; i < (int)ARRAY_SIZE(ui.ip_digit_text); ++i) {
    strncpy(ui.ip_digit_text[i], "0", 2);

    ui.ip_digit_layers[i] = text_layer_create(
        (GRect) { .origin = { 12*i, bounds.size.h / 4 }, .size = { 15, bounds.size.h / 4 } });
    text_layer_set_font(ui.ip_digit_layers[i],
                        fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(ui.ip_digit_layers[i], GTextAlignmentCenter);
    text_layer_set_text(ui.ip_digit_layers[i], ui.ip_digit_text[i]);
    layer_add_child(window_layer, text_layer_get_layer(ui.ip_digit_layers[i]));
  }

  /* Port section */

  for (int i = 0; i < (int)ARRAY_SIZE(ui.port_digit_text); ++i) {
    strncpy(ui.port_digit_text[i], "0", 2);

    ui.port_digit_layers[i] = text_layer_create(
        (GRect) { .origin = { 12*i, bounds.size.h * 3 / 4 }, .size = { 15, bounds.size.h / 4 } });
    text_layer_set_font(ui.port_digit_layers[i],
                        fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(ui.port_digit_layers[i], GTextAlignmentCenter);
    text_layer_set_text(ui.port_digit_layers[i], ui.port_digit_text[i]);
    layer_add_child(window_layer, text_layer_get_layer(ui.port_digit_layers[i]));
  }


  ui.invert_layer = inverter_layer_create((GRect) {
        .origin = { 1, bounds.size.h / 4 },
        .size = invert_layer_size
      });
  layer_add_child(window_layer, inverter_layer_get_layer(ui.invert_layer));
}

static void window_unload(Window *window)
{
	text_layer_destroy(ui.ip_title_layer);
	for (int i = 0; i < (int)ARRAY_SIZE(ui.ip_digit_text); ++i) {
		host_addr[i] = ui.ip_digit_text[i][0];
		text_layer_destroy(ui.ip_digit_layers[i]);
	}

	text_layer_destroy(ui.port_title_layer);
	for (int i = 0; i < (int)ARRAY_SIZE(ui.port_digit_text); ++i) {
		host_addr[i] = ui.port_digit_text[i][0];
		text_layer_destroy(ui.port_digit_layers[i]);
	}
	host_addr[4] = '\0';
	inverter_layer_destroy(ui.invert_layer);
	hs_callback(host_addr);
}

void entry_init(char *name) {
  ui.window = window_create();
  window_set_click_config_provider(ui.window, click_config_provider);
  window_set_window_handlers(ui.window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
}

void entry_deinit(void) {
  window_destroy(ui.window);
}

void entry_get_host_addr(EntryCallback callback) {
  hs_callback = callback;
  window_stack_push(ui.window, true);
}
