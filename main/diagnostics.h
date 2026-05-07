#pragma once

void diagnostics_init(void);
void diagnostics_emit(void);
void diagnostics_self_test(void);
void diagnostics_event_flood(int count);
void diagnostics_reset_stats(void);
void diagnostics_poll(void);
