#pragma once

typedef void (*EntryCallback)(char *addr);

void entry_init(char *name);

void entry_deinit(void);

void entry_get_host_addr(EntryCallback cb);
