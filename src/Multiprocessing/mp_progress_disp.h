
#ifndef MP_PROGRESS_DISP_H
#define MP_PROGRESS_DISP_H

#include <pthread.h>

struct print_str_s {
    char* str;
    int len;
    int task_type; // 0: log on cmd 255: kill
};

typedef struct print_str_s print_str_t;

struct progress_s {
    double best_result;
    int tasks_completed;
    int optim_mode;
};

typedef struct progress_s progress_t;

struct console_queue_s {
    int queue_size;
    int current_task_id;
    pthread_t thread_id;
    print_str_t* str_list;
    progress_t progress;
    int task_count;
    int first_task_id;
    int next_task_id;
    pthread_mutex_t* lock;
};

typedef struct console_queue_s console_queue_t;


console_queue_t init_console_queue();
void free_console_queue(console_queue_t* console_queue);

int get_print_str(console_queue_t* console_queue, print_str_t* str);

void con_printf(console_queue_t* console_queue, const char* format, ...);
void con_kill(console_queue_t* console_queue);

#endif // MP_PROGRESS_DISP_H