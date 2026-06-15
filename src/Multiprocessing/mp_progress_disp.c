
#include "mp_progress_disp.h"

console_queue_t* init_console_queue() {
    console_queue_t* console_queue = (console_queue_t*)malloc(sizeof(console_queue_t));
    if (console_queue == NULL) EXIT_MEM_ERROR();
    console_queue->queue_size = 100;
    console_queue->message_queue = (console_message_t*)malloc(sizeof(console_message_t) * console_queue->queue_size);
    if (console_queue->message_queue == NULL) EXIT_MEM_ERROR();

    console_queue->message_list_size = 10;

    console_queue->lock = (thread_mutex_t*)malloc(sizeof(thread_mutex_t));
    if (console_queue->lock == NULL) EXIT_MEM_ERROR();

    thread_mutex_init(console_queue->lock);
    console_queue->current_message_id = 0;
    console_queue->first_message_id = 0;
    console_queue->next_message_id = 0;

    return console_queue;
}

void free_console_queue(console_queue_t* console_queue) {
    if (console_queue == NULL) return;

    thread_mutex_destroy(console_queue->lock);
    free(console_queue->message_queue);
    free(console_queue->lock);
    free(console_queue);
}

static void add_to_console_queue(console_queue_t* console_queue, char* str, uint64_t len, int task_type) {
    while (1) {
        thread_mutex_lock(console_queue->lock);
        if (console_queue->first_message_id == (console_queue->next_message_id + 1) % console_queue->queue_size) {
            thread_mutex_unlock(console_queue->lock);
            Sleep(1000);
            continue;
        }
        console_queue->message_queue[console_queue->next_message_id].str = str;
        console_queue->message_queue[console_queue->next_message_id].len = len;
        console_queue->message_queue[console_queue->next_message_id].task_type = task_type;
        console_queue->next_message_id = (console_queue->next_message_id + 1) % console_queue->queue_size;
        thread_mutex_unlock(console_queue->lock);
        return;
    }
}

int get_from_console_queue(console_queue_t* console_queue, console_message_t* message) {
    thread_mutex_lock(console_queue->lock);
    if (console_queue->first_message_id == console_queue->next_message_id) {
        thread_mutex_unlock(console_queue->lock);
        return 0;
    }
    message->str = console_queue->message_queue[console_queue->first_message_id].str;
    message->len = console_queue->message_queue[console_queue->first_message_id].len;
    message->task_type = console_queue->message_queue[console_queue->first_message_id].task_type;

    console_queue->first_message_id = (console_queue->first_message_id + 1) % console_queue->queue_size;
    thread_mutex_unlock(console_queue->lock);
    return 1;
}

void con_printf(console_queue_t* console_queue, const char* format, ...) {
    // If console queue is not allocated it does not exist -> logging disabled
    if (console_queue == NULL) return;

    va_list args;
    char* formatted_str;
    int required_length;

    // Determine the length of the formatted string
    va_start(args, format);
    required_length = vsnprintf(NULL, 0, format, args) + 1; // +1 for null terminator
    va_end(args);

    // Allocate memory for the formatted string
    formatted_str = malloc(required_length);
    if (formatted_str == NULL) EXIT_MEM_ERROR();
    

    // Format the string
    va_start(args, format);
    vsnprintf(formatted_str, required_length, format, args);
    va_end(args);

    // Forward the formatted string to the buffer
    add_to_console_queue(console_queue, formatted_str, strlen(formatted_str), 0);
}