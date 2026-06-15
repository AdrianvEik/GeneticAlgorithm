
#ifndef _PROGRESS_DISPLAY_H_
#define _PROGRESS_DISPLAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../Multiprocessing/mp_progress_disp.h"
#include "../Multiprocessing/mp_logger.h"

/**
 * Render or update the terminal progress display for a running GA.
 *
 * :param ga_progress: Current aggregate progress counters and best result.
 * :param message_list_size: Number of console-message rows reserved below the
 *     progress display.
 * :param enabled: Non-zero to write ANSI terminal output; zero to do nothing.
 */
void display_progress(progress_t* ga_progress, uint32_t message_list_size, int enabled);

/**
 * Render one queued console message at a stable terminal row.
 *
 * The function owns and frees ``message`` after printing.
 *
 * :param message: Heap-allocated message string to print.
 * :param position: Message slot index below the progress display.
 * :param enabled: Non-zero to write ANSI terminal output; zero to do nothing.
 */
void display_console_message(char* message, uint32_t position, int enabled);
#endif // !_PROGRESS_DISPLAY_H_
