#include <stdio.h>
#include <stdlib.h>

// Function to display progress bar
void display_progress(int completed, int total, double current_best, double elapsed_time) {
    int bar_width = 40; // Adjust as needed
    double progress = (double)completed / total;

    // Start the progress bar
    printf("\r|<");
    int pos = (int)(bar_width * progress);

    // Fill progress bar with '#' for completed progress
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos)
            printf("#");
        else
            printf(".");
    }

    printf(">|\n");
    printf("| Current best: %.3f  |\n", current_best);
    printf("| Time: %.1f [s]       |\n", elapsed_time);
    printf("| Progress: %.2f%% [%d] |\n", progress * 100, completed);
    printf("|-----------------------|\n");

    fflush(stdout); // Ensure output updates in real-time
}