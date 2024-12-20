#include <stdio.h>
#include <stdlib.h>

// Function to display progress bar
void display_progress(int completed, int total, double current_best, double elapsed_time) {
    static int initialized = 0;
    int bar_width = 40; // Adjust as needed
    double progress = (double)completed / total;

    // Initialize display layout only once
    if (!initialized) {
        printf("\033[2J");            // Clear the screen
        printf("\033[H");             // Move cursor to the top-left corner
        printf("|<%.*s>|\n", bar_width, "........................................");
        printf("| Current best:          |\n");
        printf("| Time:                  |\n");
        printf("| Progress:              |\n");
        printf("| Time per task:         |\n");
        printf("|-----------------------|\n");
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

    printf("\033[2;18H%.3f", current_best);   // Update current best
    printf("\033[3;8H%.1f [s]       ", elapsed_time); // Update elapsed time
    printf("\033[4;12H%.2f%% [%d]   ", progress * 100, completed); // Update progress
    if (completed > 0)
        printf("\033[5;17H%.1f [s]   ", elapsed_time / completed); // Update time per task

    fflush(stdout); // Ensure the updated content is shown
}