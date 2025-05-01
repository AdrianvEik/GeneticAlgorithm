
#ifndef _PROGRESS_DISPLAY_H_
#define _PROGRESS_DISPLAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../Multiprocessing/mp_progress_disp.h"


void display_progress(console_queue_t* con_queue, int total_tasks, double elapsed_time);

#endif // !_PROGRESS_DISPLAY_H_
