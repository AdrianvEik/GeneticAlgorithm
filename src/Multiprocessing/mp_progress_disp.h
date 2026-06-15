
#ifndef MP_PROGRESS_DISP_H
#define MP_PROGRESS_DISP_H

#include <pthread.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "../Helper/Struct.h"
#include "../Helper/rng.h"
#include "../Helper/error_handling.h"

#include "mp_consts.h"

struct console_message_s {
    char* str;
    uint64_t len;
    int task_type; // 0: log on cmd 255: kill
};

typedef struct console_message_s console_message_t;

struct console_queue_s {
    uint32_t queue_size;
    uint32_t current_message_id;
    pthread_t thread_id;
    console_message_t* message_queue;
    uint32_t message_list_size;
    uint32_t message_count;
    uint32_t first_message_id;
    uint32_t next_message_id;
    pthread_mutex_t* lock;
};

typedef struct console_queue_s console_queue_t;


console_queue_t* init_console_queue();
void free_console_queue(console_queue_t* console_queue);

int get_from_console_queue(console_queue_t* console_queue, console_message_t* str);

void con_printf(console_queue_t* console_queue, const char* format, ...);
//void con_kill(console_queue_t* console_queue);

#endif // MP_PROGRESS_DISP_H