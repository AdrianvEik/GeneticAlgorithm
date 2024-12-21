
#include "mp_progress_disp.h"
#include "mp_consts.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "../Helper/Struct.h"
#include "../Helper/rng.h"


console_queue_t init_console_queue() {
    console_queue_t console_queue;
    console_queue.queue_size = 1000;
    console_queue.str_list = (print_str_t*)malloc(sizeof(print_str_t) * console_queue.queue_size);
    if (console_queue.str_list == NULL) {
        printf("Memory allocation failed: init_console_queue");
        exit(255);
    }
    console_queue.lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (console_queue.lock == NULL) {
        printf("Memory allocation failed: init_console_queue");
        exit(255);
    }
    pthread_mutex_init(console_queue.lock, NULL);
    console_queue.current_task_id = 0;
    console_queue.first_task_id = 0;
    console_queue.next_task_id = 0;

    console_queue.progress.best_result = -INFINITY;
    console_queue.progress.tasks_completed = 0;
    console_queue.progress.optim_mode = 0;
    return console_queue;
}

void free_console_queue(console_queue_t* console_queue) {
    pthread_mutex_destroy(console_queue->lock);
    free(console_queue->str_list);
    free(console_queue->lock);
}

void add_print_str(console_queue_t* console_queue, char* str, int len, int task_type) {
    int str_added = 0;
    while (!str_added) {
        pthread_mutex_lock(console_queue->lock);
        if (console_queue->first_task_id == (console_queue->next_task_id + 1) % console_queue->queue_size) {
            pthread_mutex_unlock(console_queue->lock);
            Sleep(1000);
            continue;
        }
        console_queue->str_list[console_queue->next_task_id].str = str;
        console_queue->str_list[console_queue->next_task_id].len = len;
        console_queue->str_list[console_queue->next_task_id].task_type = task_type;
        console_queue->next_task_id = (console_queue->next_task_id + 1) % console_queue->queue_size;
        str_added = 1;
        pthread_mutex_unlock(console_queue->lock);
    }
}

int get_print_str(console_queue_t* console_queue, print_str_t* str) {
    int str_retrieved = 0;
    while (!str_retrieved) {
        pthread_mutex_lock(console_queue->lock);
        if (console_queue->first_task_id == console_queue->next_task_id) {
            pthread_mutex_unlock(console_queue->lock);
            return str_retrieved;
        }
        *str = console_queue->str_list[console_queue->first_task_id];
        console_queue->first_task_id = (console_queue->first_task_id + 1) % console_queue->queue_size;
        pthread_mutex_unlock(console_queue->lock);
        str_retrieved = 1;
        return str_retrieved;
    }
}
void con_printf(console_queue_t* console_queue, const char* format, ...) {
    va_list args;
    char* formatted_str;
    int required_length;

    // Determine the length of the formatted string
    va_start(args, format);
    required_length = vsnprintf(NULL, 0, format, args) + 1; // +1 for null terminator
    va_end(args);

    // Allocate memory for the formatted string
    formatted_str = malloc(required_length);
    if (!formatted_str) {
        perror("Failed to allocate memory for formatted string: con_printf");
        return;
    }

    // Format the string
    va_start(args, format);
    vsnprintf(formatted_str, required_length, format, args);
    va_end(args);

    // Forward the formatted string to the buffer
    add_print_str(console_queue, formatted_str, strlen(formatted_str), 0);
}

void con_kill(console_queue_t* console_queue) {
    add_print_str(console_queue, "", 0, 255);
}
