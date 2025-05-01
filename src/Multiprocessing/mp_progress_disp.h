
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

struct progress_s {
    double elapsed_time;
    double best_result;
    double average_result;
    double result_standard_deviation;
    int tasks_completed;
    int optim_mode;
};

typedef struct progress_s progress_t;

struct console_queue_s {
    int queue_size;
    int current_message_id;
    pthread_t thread_id;
    console_message_t* message_queue;
    int message_list_size;
    progress_t progress;
    int message_count;
    int first_message_id;
    int next_message_id;
    pthread_mutex_t* lock;
};

typedef struct console_queue_s console_queue_t;


console_queue_t init_console_queue();
void free_console_queue(console_queue_t* console_queue);

int get_from_console_queue(console_queue_t* console_queue, console_message_t* str);

void con_printf(console_queue_t* console_queue, const char* format, ...);
void con_kill(console_queue_t* console_queue);

#endif // MP_PROGRESS_DISP_H