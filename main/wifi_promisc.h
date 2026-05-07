#pragma once
#include <stdbool.h>

void wifi_promisc_init(void);
void wifi_promisc_start(void);
void wifi_promisc_stop(void);
void wifi_promisc_poll(void);
bool wifi_promisc_is_enabled(void);
