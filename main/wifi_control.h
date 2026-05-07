#pragma once

void wifi_control_init(void);
void wifi_control_radio_on(void);
void wifi_control_radio_off(void);
void wifi_control_set_channel(int channel);
void wifi_control_get_mac(void);
void wifi_control_set_country(const char *cc);
