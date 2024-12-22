#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>

double get_elapsed_time(struct timespec end_time, struct timespec start_time) {
        return (end_time.tv_sec - start_time.tv_sec) +
                (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

}

int main(int argc, char *argv[]) {
        if (argc < 2) {
                printf("Used: %s <command> [args...n]", argv[0]);
                return 1;
        }
        
        char **args = &argv[1]; // bypass the name of programm
        int pipefd[2];
        pid_t pid;

        if (pipe(pipefd) == -1) {
                perror("Pipe creation failed.");
                return 1;
        }

        pid = fork();

        if (pid < 0) {
                perror("Fork Failed");
                return 1;
        }
        if (pid == 0) {
                close(pipefd[0]);
                struct timespec start_time;
                clock_gettime(CLOCK_MONOTONIC, &start_time);
                if (write(pipefd[1], &start_time, sizeof(start_time)) == -1) {
                        perror("Write failed");
                } else {
                        if (execvp(args[0], args) == -1) {
                                perror("Exec failed");
                                close(pipefd[1]);
                                exit(1);
                        }
                }
        }

        if (pid > 0) {
                close(pipefd[1]);
                struct timespec start_time, end_time;
                if (read(pipefd[0], &start_time, sizeof(start_time)) == -1) {
                        perror("Read Failed\n");
                }
                wait(NULL);
                clock_gettime(CLOCK_MONOTONIC, &end_time);
                close(pipefd[1]);

                double elapsed_time = get_elapsed_time(end_time, start_time);

                printf("Elapsed time: %.9f seconds\n", elapsed_time);
        }


        return 0;
}