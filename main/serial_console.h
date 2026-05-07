#pragma once
void serial_console_init(void);
int serial_console_readline(char *buf, int max_len);
void serial_console_write(const char *s);
