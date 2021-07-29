#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int             is_debug = 0;

static void get_current_time(char* buffer, int len) {
    time_t timer;
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", tm_info);
}

static void print(char* msg) {
    char time_str[26];
    get_current_time(time_str, 26);
    printf("%s: %s", time_str, msg);
}

void set_log_level(int lvl) {
    is_debug = lvl;
}

// print debug
void printd(char* msg) {
    if(is_debug) {
        print(msg);
    }
    return;
}

// print error
void printe(char* msg) {
    print(msg);
}