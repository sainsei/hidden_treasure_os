#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>

#define MATRIX_SIZE 4
#define MAX_RANDOM_NUMBER 100

// Define the matrix, player scores, mutex locks, and the count of unopened cells
int matrix[MATRIX_SIZE][MATRIX_SIZE];
bool cell_opened[MATRIX_SIZE][MATRIX_SIZE];
int even_score = 0;
int odd_score = 0;
pthread_mutex_t even_lock;
pthread_mutex_t odd_lock;
pthread_mutex_t unopened_cells_lock;
int unopened_cells;
int turn; // 0 for even player and 1 for odd player

// Function to log game events
void log_event(char *message)
{
    FILE *log_file = fopen("game_log.txt", "a");
    if (log_file != NULL)
    {
        fprintf(log_file, "%s\n", message);
        fclose(log_file);
    }
}

// Function to find and open a random unopened cell
void open_random_cell(int *row, int *col)
{
    do
    {
        *row = rand() % MATRIX_SIZE;
        *col = rand() % MATRIX_SIZE;
    } while (cell_opened[*row][*col]);

    cell_opened[*row][*col] = true;
    pthread_mutex_lock(&unopened_cells_lock);
    unopened_cells--;
    pthread_mutex_unlock(&unopened_cells_lock);
}

// Function for Even player's turn
void *even_player_turn(void *arg)
{
    while (true)
    {
        //check if there is unopened cell is left or not
        pthread_mutex_lock(&unopened_cells_lock);
        if (unopened_cells == 0)
        {
            pthread_mutex_unlock(&unopened_cells_lock);
            break;
        }
        pthread_mutex_unlock(&unopened_cells_lock);

        //check if it is even player's turn or not
        if (turn == 0)
        {
            int row, col;
            open_random_cell(&row, &col);

            int number;
            pthread_mutex_lock(&even_lock);
            number = matrix[row][col];
            pthread_mutex_unlock(&even_lock);

            if (number % 2 == 0)
            {
                pthread_mutex_lock(&even_lock);
                even_score += number;
                pthread_mutex_unlock(&even_lock);
            }
            else
            {
                pthread_mutex_lock(&odd_lock);
                even_score -= number;
                pthread_mutex_unlock(&odd_lock);
            }

            // Log the event
            char event_message[100];
            snprintf(event_message, sizeof(event_message), "Even player opened cell [%d, %d] with value %d", row, col, number);
            log_event(event_message);
            snprintf(event_message, sizeof(event_message), "Even Player Score: %d, Odd Player Score: %d", even_score, odd_score);
            log_event(event_message);

            turn = 1;  //change the turn to odd
        }
    }

    return NULL;
}

// Function for Odd player's turn
void *odd_player_turn(void *arg)
{
    while (true)
    {
        pthread_mutex_lock(&unopened_cells_lock);
        if (unopened_cells == 0)
        {
            pthread_mutex_unlock(&unopened_cells_lock);
            break;
        }
        pthread_mutex_unlock(&unopened_cells_lock);

        if (turn == 1)
        {
            int row, col;
            open_random_cell(&row, &col);

            int number;
            pthread_mutex_lock(&odd_lock);
            number = matrix[row][col];
            pthread_mutex_unlock(&odd_lock);

            if (number % 2 != 0)
            {
                pthread_mutex_lock(&odd_lock);
                odd_score += number;
                pthread_mutex_unlock(&odd_lock);
            }
            else
            {
                pthread_mutex_lock(&even_lock);
                odd_score -= number;
                pthread_mutex_unlock(&even_lock);
            }

            // Log the event
            char event_message[100];
            snprintf(event_message, sizeof(event_message), "Odd player opened cell [%d, %d] with value %d", row, col, number);
            log_event(event_message);
            snprintf(event_message, sizeof(event_message), "Even Player Score: %d, Odd Player Score: %d", even_score, odd_score);
            log_event(event_message);

            turn = 0; // change the turn to even
        }
    }

    return NULL;
}

int main()
{

    sleep(1);

    // Seed the random number generator
    unsigned int seed = (unsigned int)time(NULL);
    srand(seed);

    // Log the start of the game
    log_event("Game started.");

    // Initialize the matrix with random positive integers
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            matrix[i][j] = rand_r(&seed) % MAX_RANDOM_NUMBER + 1;
            cell_opened[i][j] = false;
        }
    }

    // Initialize the count of unopened cells
    unopened_cells = MATRIX_SIZE * MATRIX_SIZE;

    // Initialize mutex locks
    pthread_mutex_init(&odd_lock, NULL);
    pthread_mutex_init(&even_lock, NULL);
    pthread_mutex_init(&unopened_cells_lock, NULL);

    // Create threads for Even and Odd players
    pthread_t even_thread, odd_thread;

    //randomly choosing which player will start the game
    turn = rand() % 2;
    if (turn == 0)  //even will start
    {
        log_event("Even Player started the game.");
        pthread_create(&even_thread, NULL, even_player_turn, NULL);
        pthread_create(&odd_thread, NULL, odd_player_turn, NULL);
    }
    else 
    {
        log_event("Odd Player started the game.");
        pthread_create(&odd_thread, NULL, odd_player_turn, NULL);
        pthread_create(&even_thread, NULL, even_player_turn, NULL);
    }

    // Wait for the threads to finish
    pthread_join(odd_thread, NULL);
    pthread_join(even_thread, NULL);

    // Determine the winner based on scores
    if (even_score > odd_score)
    {
        log_event("Even player wins!");
    }
    else if (odd_score > even_score)
    {
        log_event("Odd player wins!");
    }
    else
    {
        log_event("It's a tie!");
    }

    // Clean up mutex locks
    pthread_mutex_destroy(&even_lock);
    pthread_mutex_destroy(&odd_lock);
    pthread_mutex_destroy(&unopened_cells_lock);

    FILE* log_file = fopen("game_log.txt", "a");
    fprintf(log_file, "Opened Matrix: \n");
    for(int i=0; i<MATRIX_SIZE; i++){
        for(int j=0; j<MATRIX_SIZE; j++){
            fprintf(log_file, "%d ", matrix[i][j]);
        }
        fprintf(log_file, "\n");
    }
    fclose(log_file);

    // Log the end of the game
    log_event("Game ended.");
    log_event("\n");

    return 0;
}
