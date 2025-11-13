
#ifndef _PROGRESS_DISPLAY_H_
#define _PROGRESS_DISPLAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../Multiprocessing/mp_progress_disp.h"
#include "../Multiprocessing/mp_logger.h"

void display_progress(progress_t* ga_progress, int message_list_size);
void display_console_message(char* message, int position);
#endif // !_PROGRESS_DISPLAY_H_
