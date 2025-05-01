
#ifndef _PROGRESS_DISPLAY_H_
#define _PROGRESS_DISPLAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../Multiprocessing/mp_progress_disp.h"


void display_progress(console_queue_t* con_queue);
void display_console_message(char* message, int position);
#endif // !_PROGRESS_DISPLAY_H_
