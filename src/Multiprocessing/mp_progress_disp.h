
#ifndef MP_PROGRESS_DISP_H
#define MP_PROGRESS_DISP_H

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
#include "mp_thread_wrapper.h"


/**
 * One console message queued for display by the logging thread.
 *
 * ``task_type`` uses the same termination convention as the other queues:
 * regular log messages use ``0`` and termination messages use
 * ``TERMINATE_THREAD``.
 */
struct console_message_s {
    /** Heap-allocated formatted message string. */
    char* str;
    /** String length in bytes, excluding the null terminator. */
    uint64_t len;
    /** Message kind, currently normal log output or termination. */
    int task_type;
};

typedef struct console_message_s console_message_t;

/**
 * Ring buffer used to pass console messages into the display/logging thread.
 *
 * Solver threads call :c:func:`con_printf`; the logging thread drains the
 * queue with :c:func:`get_from_console_queue` and renders messages through
 * the progress-display module.
 */
struct console_queue_s {
    /** Number of message slots in the ring buffer. */
    uint32_t queue_size;
    /** Next message id assigned by producers. */
    uint32_t current_message_id;
    /** Optional display/logging thread handle. */
    thread_t thread_id;
    /** Ring-buffer storage for console messages. */
    console_message_t* message_queue;
    /** Number of message rows reserved in the terminal display. */
    uint32_t message_list_size;
    /** Expected number of solver-task messages. */
    uint32_t message_count;
    /** Read cursor in ``message_queue``. */
    uint32_t first_message_id;
    /** Write cursor in ``message_queue``. */
    uint32_t next_message_id;
    /** Mutex protecting queue cursors and storage. */
    thread_mutex_t* lock;
};

typedef struct console_queue_s console_queue_t;


/**
 * Allocate and initialize a console-message queue.
 *
 * :returns: Heap-allocated queue. Release it with
 *     :c:func:`free_console_queue`.
 */
console_queue_t* init_console_queue();

/**
 * Release a console-message queue and any queued message strings.
 *
 * :param console_queue: Queue returned by :c:func:`init_console_queue`.
 */
void free_console_queue(console_queue_t* console_queue);

/**
 * Try to pop one console message.
 *
 * :param console_queue: Queue to read from.
 * :param str: Destination message struct filled on success.
 * :returns: ``1`` when a message was read, otherwise ``0``.
 */
int get_from_console_queue(console_queue_t* console_queue, console_message_t* str);

/**
 * Format and enqueue a console message.
 *
 * :param console_queue: Queue receiving the formatted message.
 * :param format: ``printf``-style format string.
 */
void con_printf(console_queue_t* console_queue, const char* format, ...);
//void con_kill(console_queue_t* console_queue);

#endif // MP_PROGRESS_DISP_H
