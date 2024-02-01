#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MAX_PATHS 10
#define MAX_PATH_LENGTH 256
#define MAX_ARGS 32

int main(int argc, char* argv[]){
    char error_message[] = "An error has occurred\n";
    char *PATH[MAX_PATHS] = {"/bin/"};
    // strcat(PATH, "/");
    char buffer[BUFFER_SIZE];

    printf("wish> ");

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL)
    {
        if (strcmp(buffer, "\n") == 0){
            printf("wish> ");
            continue;
        }
        // Remove newline character from the end of the input
        buffer[strcspn(buffer, "\n")] = '\0';

        // printf("buffer %s\n", buffer);

        char* token;
        char* args[MAX_ARGS] = {NULL}; // Initialize args with NULL pointers
        int arg_count = 0;
        // strtok() returns a pointer
        token = strtok(buffer, " ");
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            // printf("token %s\n", token);
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }

        if (strcmp(args[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(args[0], "cd") == 0) {
            // Change directory
            if (arg_count != 2) {
                // Invalid number of arguments
                write(STDERR_FILENO, error_message, strlen(error_message));
            } else {
                if (chdir(args[1]) != 0) {
                    // chdir failed
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }
        } else if (strcmp(args[0], "path") == 0) {
            if (arg_count == 1) {
                printf("wish> ");
                continue;
            } else {
                // Clear existing PATH
                for (int i = 0; i < MAX_PATHS; i++) {
                    PATH[i] = NULL;
                }
                // Set new PATH
                for (int j = 1; j < arg_count && j < MAX_PATHS; j++) {
                    // Append '/' to the end of the directory path
                    size_t len = strlen(args[j]);
                    if (len > 0 && args[j][len-1] != '/'){
                        strcat(args[j], "/");
                    }
                    PATH[j - 1] = args[j];
                }
            }
        } else {
            pid_t pid = fork();

            if (pid < 0){
                // Fork failed
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0){
                // Child process
                int path_success = 0;
                for (int j = 0; j < MAX_PATHS; j++) {
                    char command_path[MAX_PATH_LENGTH];
                    snprintf(command_path, MAX_PATH_LENGTH, "%s%s", PATH[j], args[0]);
                    if (access(command_path, X_OK) == 0){
                        path_success = 1;
                        execv(command_path, args);
                        // If execv returns it must've failed
                        perror("execv");
                        exit(EXIT_FAILURE);
                    }
                }
                if (!path_success) {
                    perror("command not found");
                    exit(EXIT_FAILURE);
                }
            } else {
                // Parent process
                int status;
                waitpid(pid, &status, 0);
            }
        }
        printf("wish> ");
    }
    
    return 0;
}