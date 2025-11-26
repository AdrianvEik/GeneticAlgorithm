
#include "progress_display.h"

// Function to display progress bar
void display_progress(progress_t* ga_progress, int message_list_size, int enabled) {
    if (!enabled) return;

    static int initialized = 0;
    //static int saved_cursor = 0;
    int bar_width = 40; // Adjust as needed

    double progress = (double) ga_progress->tasks_completed / ga_progress->max_tasks;
    double average_result = ga_progress->average_result / ga_progress->tasks_completed;
    double average_standard_deviation = sqrt(ga_progress->result_standard_deviation / (ga_progress->tasks_completed-1));


    if (!initialized) {


        // Initialize display layout
        //printf("\033[2J");            // Clear the screen
        printf("\033[H");             // Move cursor to the top-left corner
        printf("\033[1;1H|<%.*s>|\n", bar_width, "........................................");
        printf("\033[2;1H| Current best:                            |\n");
        printf("\033[3;1H| Average best:                            |\n");
        printf("\033[4;1H| Time:                                    |\n");
        printf("\033[5;1H| Progress:                                |\n");
        printf("\033[6;1H| Time per task:                           |\n");
        printf("\033[7;1H|------------------------------------------|\n");
        for (int i = 0; i < message_list_size +2; ++i) {
            printf("\n");
        }
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

    printf("\033[2;18H%.3f", ga_progress->best_result);   // Update current best
    printf("\033[3;18H%.3f e: %.3f", average_result, average_standard_deviation);     // average and standard deviation
    printf("\033[4;8H%.3f [s]       ", ga_progress->elapsed_time); // Update elapsed time
    printf("\033[5;12H%.2f%% [%d]   ", progress * 100, ga_progress->tasks_completed); // Update progress
    if (ga_progress->tasks_completed > 0)
        printf("\033[6;17H%.3f [s]   ", ga_progress->elapsed_time / ga_progress->tasks_completed); // Update time per task

    printf("\033[%d;1H", message_list_size + 10); // Move cursor to the next line
    fflush(stdout);

}

void display_console_message(char* message, int position, int enabled){
    if (!enabled) return;
    printf("\033[%d;1H%s", position + 9, message); // Update messages
    printf("\033[%d;1H\033[2K", position + 10); // clear line below

    free(message);
}

