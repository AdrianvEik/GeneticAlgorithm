#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "progress_display.h"
#include "../Multiprocessing/mp_progress_disp.h"

// Function to finalize progress display and restore cursor
void finalize_progress_display(int initialized) {
    if (!initialized) return;

    // Restore the saved cursor position
    printf("\033[u"); // Restore cursor position
    printf("\033[0J"); // Clear everything below the cursor
    fflush(stdout);
}

// Function to display progress bar
void display_progress(console_queue_t* con_queue, int total_tasks, double elapsed_time) {
    static int initialized = 0;
    static int saved_cursor = 0;
    int bar_width = 40; // Adjust as needed
    double progress = (double) con_queue->progress.tasks_completed / total_tasks;
    double average_result = con_queue->progress.average_result / con_queue->progress.tasks_completed;
    double average_standard_deviation = sqrt(con_queue->progress.result_standard_deviation / (con_queue->progress.tasks_completed-1));


    if (!initialized) {
        // Save the current cursor position
        printf("\033[s"); // Save cursor position
        saved_cursor = 1;

        // Initialize display layout
        //printf("\033[2J");            // Clear the screen
        printf("\033[H");             // Move cursor to the top-left corner
        printf("|<%.*s>|\n", bar_width, "........................................");
        printf("| Current best:                         |\n");
        printf("| Average best:                         |\n");
        printf("| Time:                                 |\n");
        printf("| Progress:                             |\n");
        printf("| Time per task:                        |\n");
        printf("|---------------------------------------|\n");
        fflush(stdout);
        initialized = 1;
    }

    // Update only the necessary parts
    printf("\033[1;3H"); // Move to the first line, 3rd character
    int pos = (int)(bar_width * progress);
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos)
            printf("#");
        else
            printf(".");
    }

    printf("\033[2;18H%.3f", con_queue->progress.best_result);   // Update current best
    printf("\033[3;18H%.3f e: %.3f", average_result, average_standard_deviation);     // average and standard deviation
    printf("\033[4;8H%.3f [s]       ", elapsed_time); // Update elapsed time
    printf("\033[5;12H%.2f%% [%d]   ", progress * 100, con_queue->progress.tasks_completed); // Update progress
    if (con_queue->progress.tasks_completed > 0)
        printf("\033[6;17H%.3f [s]   ", elapsed_time / con_queue->progress.tasks_completed); // Update time per task

    
    finalize_progress_display(initialized);
}

