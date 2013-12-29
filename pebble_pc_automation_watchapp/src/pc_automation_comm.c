#include "pc_automation.h"

enum {
	KEY_ROW_SELECTED = 0,
	KEY_SERVER_SUCCESS,
	KEY_SERVER_ERROR,
	KEY_SERVER_ADDR,
};

// outgoing message was delivered
void out_sent_handler(DictionaryIterator *sent, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sent!");
}

// outgoing message failed
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context)
{
	char buf[20];

	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send! Reason: %d", reason);
	snprintf(buf, sizeof(buf), "out_failed %d", reason);
	update_row_subtitle(get_last_row_clicked(), buf);
}


// incoming message received
void in_received_handler(DictionaryIterator *received, void *context)
{
	Tuple *srv_result_tuple;
	char buf[32];
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Received!");

	srv_result_tuple = dict_find(received, KEY_SERVER_SUCCESS);
	if (srv_result_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing Ok! or clearing subtitle");
#if DEBUG
		update_row_subtitle(get_last_row_clicked(), "Ok!");
#else
		update_row_subtitle(get_last_row_clicked(), "");
#endif
		return;
	}

	srv_result_tuple = dict_find(received, KEY_SERVER_ERROR);
	if (srv_result_tuple) {
		strncpy(buf, srv_result_tuple->value->cstring, sizeof(buf));
		update_row_subtitle(get_last_row_clicked(), buf);
	}

	srv_result_tuple = dict_find(received, KEY_SERVER_ADDR);
	if (srv_result_tuple) {
		strncpy(buf, srv_result_tuple->value->cstring, sizeof(buf));
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Got server address: %s", buf);
		update_row_subtitle(0 /*Settings row*/, buf);
		return;
	}
}


// incoming message dropped
void in_dropped_handler(AppMessageResult reason, void *context)
{
	char buf[20];

	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped! Reason: %d", reason);
	snprintf(buf, sizeof(buf), "in_drop %d", reason);
	update_row_subtitle(get_last_row_clicked(), buf);
}

void run_pc_script(int row_num)
{
	Tuplet row_sel_tuple = TupletInteger(KEY_ROW_SELECTED, row_num);
	DictionaryIterator *iter;

	app_message_outbox_begin(&iter);
	if (iter == NULL)
		return;

	dict_write_tuplet(iter, &row_sel_tuple );
	dict_write_end(iter);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing Seding...");
	update_row_subtitle(get_last_row_clicked(), "Sending...");
	app_message_outbox_send();
}

void get_server_addr()
{
	Tuplet row_sel_tuple = TupletInteger(KEY_SERVER_ADDR, 1);
	DictionaryIterator *iter;

	app_message_outbox_begin(&iter);
	if (iter == NULL)
		return;

	dict_write_tuplet(iter, &row_sel_tuple );
	dict_write_end(iter);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting server address...");
	app_message_outbox_send();
}

void app_message_init()
{
	const uint32_t inbound_size = 64;
	const uint32_t outbound_size = 64;

	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
	app_message_open(inbound_size, outbound_size);
}
