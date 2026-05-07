#include "serial_console.h"
#include <stdio.h>

void serial_console_init(void) { setvbuf(stdin, NULL, _IONBF, 0); setvbuf(stdout, NULL, _IONBF, 0); }

int serial_console_readline(char *buf, int max_len) {
    if (!fgets(buf, max_len, stdin)) return 0;
    return 1;
}

void serial_console_write(const char *s) { printf("%s\n", s); }
