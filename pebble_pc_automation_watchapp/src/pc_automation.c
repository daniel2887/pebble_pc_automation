#include "pc_automation.h"

#define NUM_MENU_SECTIONS 1

#define SETTINGS_ROW 0
#define PREV_ROW 1
#define PLAYPAUSE_ROW 2
#define NEXT_ROW 3
#if DEBUG
#define DEBUG_ROW 4
#endif

static Window *window;
MenuLayer *menu_layer;

const char row_titles[NUM_FIRST_MENU_ITEMS][32] = {
	"Settings",
	"Previous Track",
	"Play/Pause",
	"Next Track",
	"Vol Up",
	"Vol Down",
	"Mute",
#if DEBUG
	"Debug",
#endif
};

char row_subtitles[NUM_FIRST_MENU_ITEMS][128] = {
	"", "", "", "", "", "", "",
#if DEBUG
	"",
#endif
};

int last_row_clicked = -1;

int get_last_row_clicked() { return last_row_clicked; }

void update_row_subtitle(int row_num, const char *subtitle)
{
	snprintf(row_subtitles[row_num], sizeof(row_subtitles[row_num]), subtitle);
	layer_mark_dirty(menu_layer_get_layer(menu_layer));
}

// A callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
	return NUM_MENU_SECTIONS;
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
	switch (section_index) {
		case 0:
			return NUM_FIRST_MENU_ITEMS;

		default:
			return 0;
	}
}

// A callback is used to specify the height of the section header
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
	// This is a define provided in pebble.h that you may use for the default height
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

// Here we draw what each header is
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
	// Determine which section we're working with
	switch (section_index) {
		case 0:
			// Draw title text in the section header
			menu_cell_basic_header_draw(ctx, cell_layer, "PC Automation Menu");
			break;

		default:
			menu_cell_basic_header_draw(ctx, cell_layer, "Error???");
			break;
	}
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
	menu_cell_basic_draw(ctx, cell_layer, row_titles[cell_index->row], row_subtitles[cell_index->row], NULL);
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
	last_row_clicked = cell_index->row;
#if DEBUG
	snprintf(row_subtitles[NUM_FIRST_MENU_ITEMS-1], sizeof(row_subtitles[NUM_FIRST_MENU_ITEMS-1]), "Row clicked: %d", last_row_clicked);
	layer_mark_dirty(menu_layer_get_layer(menu_layer));
#endif

	/* First row is Settings - special case */
	if (cell_index->row == 0)
		return;
	else
		run_pc_script(cell_index->row);
}

static void window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// Create the menu layer, convering the entire window
	menu_layer = menu_layer_create(bounds);

	// Set all the callbacks for the menu layer
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
		});

	menu_layer_set_click_config_onto_window(menu_layer, window);

	get_server_addr();

	layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window)
{
	menu_layer_destroy(menu_layer);
}

static void init(void)
{
	window = window_create();

	app_message_init();

	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
		});
	const bool animated = true;
	window_stack_push(window, animated);
}

static void deinit(void)
{
	window_destroy(window);
}

int main(void)
{
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}
